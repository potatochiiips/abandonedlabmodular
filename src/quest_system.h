#pragma once
#include "globals.h"
#include <vector>
#include <string>
#include <functional>

// Quest objectives
enum QuestObjectiveType {
    QUEST_OBJ_COLLECT,
    QUEST_OBJ_REACH_LOCATION,
    QUEST_OBJ_DEFEAT_ENEMY,
    QUEST_OBJ_CRAFT_ITEM
};

struct QuestObjective {
    QuestObjectiveType type;
    int targetId; // Item ID, location ID, enemy type, etc.
    int targetCount;
    int currentCount;
    std::string description;
};

// Quest structure
struct Quest {
    int id;
    std::string name;
    std::string description;
    std::vector<QuestObjective> objectives;
    int xpReward;
    bool isActive;
    bool isCompleted;
};

// Player progression system
class PlayerProgression {
public:
    PlayerProgression() : level(1), currentXP(0), skillPoints(0) {
        CalculateXPForNextLevel();
    }
    
    int GetLevel() const { return level; }
    int GetCurrentXP() const { return currentXP; }
    int GetXPForNextLevel() const { return xpForNextLevel; }
    int GetSkillPoints() const { return skillPoints; }
    
    // Add XP and handle level-ups
    void AddXP(int amount) {
        currentXP += amount;
        
        while (currentXP >= xpForNextLevel) {
            LevelUp();
        }
    }
    
    // Use skill points
    bool UseSkillPoint() {
        if (skillPoints > 0) {
            skillPoints--;
            return true;
        }
        return false;
    }
    
    // Save/Load
    void SaveToFile(std::ofstream& file) {
        file << "level " << level << "\n";
        file << "xp " << currentXP << "\n";
        file << "skillpoints " << skillPoints << "\n";
    }
    
    void LoadFromFile(std::ifstream& file) {
        std::string key;
        while (file >> key) {
            if (key == "level") file >> level;
            else if (key == "xp") file >> currentXP;
            else if (key == "skillpoints") file >> skillPoints;
            else break; // Exit when we hit non-progression data
        }
        CalculateXPForNextLevel();
    }
    
private:
    int level;
    int currentXP;
    int xpForNextLevel;
    int skillPoints;
    
    void LevelUp() {
        level++;
        currentXP -= xpForNextLevel;
        skillPoints += 5;
        CalculateXPForNextLevel();
        TraceLog(LOG_INFO, TextFormat("LEVEL UP! Now level %d. Gained 5 skill points!", level));
    }
    
    void CalculateXPForNextLevel() {
        // Exponential scaling: 100 * level^1.5
        xpForNextLevel = (int)(100.0f * powf((float)level, 1.5f));
    }
};

// Quest manager
class QuestManager {
public:
    QuestManager() : nextQuestId(0) {}
    
    // Add a quest
    int AddQuest(const std::string& name, const std::string& description, int xpReward) {
        Quest q;
        q.id = nextQuestId++;
        q.name = name;
        q.description = description;
        q.xpReward = xpReward;
        q.isActive = true;
        q.isCompleted = false;
        quests.push_back(q);
        return q.id;
    }
    
    // Add objective to quest
    void AddObjective(int questId, QuestObjectiveType type, int targetId, int targetCount, const std::string& desc) {
        for (auto& q : quests) {
            if (q.id == questId && !q.isCompleted) {
                QuestObjective obj;
                obj.type = type;
                obj.targetId = targetId;
                obj.targetCount = targetCount;
                obj.currentCount = 0;
                obj.description = desc;
                q.objectives.push_back(obj);
                break;
            }
        }
    }
    
    // Update quest progress
    void UpdateProgress(int questId, int objectiveIndex, int amount = 1) {
        for (auto& q : quests) {
            if (q.id == questId && !q.isCompleted && q.isActive) {
                if (objectiveIndex < (int)q.objectives.size()) {
                    q.objectives[objectiveIndex].currentCount += amount;
                    CheckQuestCompletion(q);
                }
                break;
            }
        }
    }
    
    // Get active quests
    std::vector<Quest*> GetActiveQuests() {
        std::vector<Quest*> active;
        for (auto& q : quests) {
            if (q.isActive && !q.isCompleted) {
                active.push_back(&q);
            }
        }
        return active;
    }
    
    // Complete quest and award XP
    bool CompleteQuest(int questId, PlayerProgression& progression) {
        for (auto& q : quests) {
            if (q.id == questId && !q.isCompleted) {
                if (IsQuestComplete(q)) {
                    q.isCompleted = true;
                    progression.AddXP(q.xpReward);
                    TraceLog(LOG_INFO, TextFormat("Quest '%s' completed! +%d XP", q.name.c_str(), q.xpReward));
                    return true;
                }
            }
        }
        return false;
    }
    
    // Draw quest tracker HUD
    void DrawQuestTracker(int screenW, int screenH) {
        auto activeQuests = GetActiveQuests();
        if (activeQuests.empty()) return;
        
        int trackerX = screenW - 320;
        int trackerY = 60;
        int trackerW = 300;
        
        DrawRectangle(trackerX, trackerY, trackerW, 30, Color{0, 0, 0, 180});
        DrawText("ACTIVE QUESTS", trackerX + 10, trackerY + 8, 16, PIPBOY_GREEN);
        
        int yOffset = 35;
        for (const auto* quest : activeQuests) {
            int questH = 25 + (int)quest->objectives.size() * 20;
            DrawRectangle(trackerX, trackerY + yOffset, trackerW, questH, Color{0, 0, 0, 160});
            DrawRectangleLines(trackerX, trackerY + yOffset, trackerW, questH, PIPBOY_GREEN);
            
            DrawText(quest->name.c_str(), trackerX + 10, trackerY + yOffset + 5, 14, PIPBOY_GREEN);
            
            int objY = trackerY + yOffset + 25;
            for (const auto& obj : quest->objectives) {
                Color objColor = (obj.currentCount >= obj.targetCount) ? Color{50, 255, 50, 255} : PIPBOY_DIM;
                DrawText(TextFormat("[%d/%d] %s", obj.currentCount, obj.targetCount, obj.description.c_str()),
                         trackerX + 15, objY, 12, objColor);
                objY += 20;
            }
            
            yOffset += questH + 10;
        }
    }
    
private:
    std::vector<Quest> quests;
    int nextQuestId;
    
    bool IsQuestComplete(const Quest& q) {
        for (const auto& obj : q.objectives) {
            if (obj.currentCount < obj.targetCount) return false;
        }
        return true;
    }
    
    void CheckQuestCompletion(Quest& q) {
        if (IsQuestComplete(q)) {
            TraceLog(LOG_INFO, TextFormat("Quest '%s' objectives complete! Turn in to complete.", q.name.c_str()));
        }
    }
};

// Skill tree definition
enum SkillType {
    SKILL_HEALTH_BOOST,
    SKILL_STAMINA_BOOST,
    SKILL_WEAPON_DAMAGE,
    SKILL_RELOAD_SPEED,
    SKILL_MOVEMENT_SPEED,
    SKILL_CARRY_CAPACITY,
    SKILL_CRAFTING_EFFICIENCY,
    SKILL_SCAVENGING,
    SKILL_COUNT
};

struct Skill {
    SkillType type;
    std::string name;
    std::string description;
    int currentLevel;
    int maxLevel;
    int cost; // Skill points per level
};

class SkillTree {
public:
    SkillTree() {
        InitializeSkills();
    }
    
    void InitializeSkills() {
        skills.push_back({SKILL_HEALTH_BOOST, "Vitality", "Increases max health by 10 per level", 0, 5, 1});
        skills.push_back({SKILL_STAMINA_BOOST, "Endurance", "Increases max stamina by 10 per level", 0, 5, 1});
        skills.push_back({SKILL_WEAPON_DAMAGE, "Marksman", "Increases weapon damage by 5% per level", 0, 5, 1});
        skills.push_back({SKILL_RELOAD_SPEED, "Quick Hands", "Reduces reload time by 10% per level", 0, 5, 1});
        skills.push_back({SKILL_MOVEMENT_SPEED, "Runner", "Increases movement speed by 5% per level", 0, 5, 1});
        skills.push_back({SKILL_CARRY_CAPACITY, "Pack Mule", "Increases inventory slots by 3 per level", 0, 3, 2});
        skills.push_back({SKILL_CRAFTING_EFFICIENCY, "Engineer", "10% chance per level to save materials when crafting", 0, 5, 1});
        skills.push_back({SKILL_SCAVENGING, "Scavenger", "Increased item find chance by 10% per level", 0, 5, 1});
    }
    
    bool UpgradeSkill(SkillType type, PlayerProgression& progression) {
        for (auto& skill : skills) {
            if (skill.type == type) {
                if (skill.currentLevel < skill.maxLevel && progression.UseSkillPoint()) {
                    skill.currentLevel++;
                    TraceLog(LOG_INFO, TextFormat("Upgraded %s to level %d", skill.name.c_str(), skill.currentLevel));
                    return true;
                }
            }
        }
        return false;
    }
    
    int GetSkillLevel(SkillType type) const {
        for (const auto& skill : skills) {
            if (skill.type == type) return skill.currentLevel;
        }
        return 0;
    }
    
    const std::vector<Skill>& GetSkills() const {
        return skills;
    }
    
private:
    std::vector<Skill> skills;
};

// Global instances
extern PlayerProgression g_PlayerProgression;
extern QuestManager g_QuestManager;
extern SkillTree g_SkillTree;