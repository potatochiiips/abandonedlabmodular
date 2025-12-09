#include "ui_tabs.h"
#include "quest_system.h"

// Static selection tracking
static int skillSelection = 0;
static int questSelection = 0;

// Draw skills screen
void DrawSkillsScreen(int screenW, int screenH, int contentX, int contentY, int contentW, int contentH, bool useController) {
    // Header - show player level and skill points
    DrawText(TextFormat("LEVEL: %d", g_PlayerProgression.GetLevel()), 
             contentX + 20, contentY + 10, 24, PIPBOY_GREEN);
    
    DrawText(TextFormat("XP: %d / %d", g_PlayerProgression.GetCurrentXP(), 
             g_PlayerProgression.GetXPForNextLevel()), 
             contentX + 200, contentY + 10, 20, PIPBOY_DIM);
    
    DrawText(TextFormat("SKILL POINTS: %d", g_PlayerProgression.GetSkillPoints()), 
             contentX + 500, contentY + 10, 22, Color{255, 215, 0, 255});
    
    // XP bar
    int barW = contentW - 40;
    int barH = 20;
    int barX = contentX + 20;
    int barY = contentY + 45;
    
    float xpProgress = (float)g_PlayerProgression.GetCurrentXP() / 
                       (float)g_PlayerProgression.GetXPForNextLevel();
    
    DrawRectangle(barX, barY, barW, barH, PIPBOY_DARK);
    DrawRectangle(barX, barY, (int)(barW * xpProgress), barH, PIPBOY_GREEN);
    DrawRectangleLines(barX, barY, barW, barH, PIPBOY_GREEN);
    
    // Skills list
    int skillY = contentY + 80;
    const auto& skills = g_SkillTree.GetSkills();
    
    // Navigation
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        skillSelection = (skillSelection - 1 + (int)skills.size()) % (int)skills.size();
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        skillSelection = (skillSelection + 1) % (int)skills.size();
    }
    
    // Draw skills
    for (size_t i = 0; i < skills.size(); i++) {
        const Skill& skill = skills[i];
        
        bool isSelected = (skillSelection == (int)i);
        Color bgColor = isSelected ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color textColor = skill.currentLevel > 0 ? PIPBOY_GREEN : PIPBOY_DIM;
        
        int skillH = 70;
        int skillX = contentX + 20;
        
        DrawRectangle(skillX, skillY, contentW - 40, skillH, bgColor);
        DrawRectangleLines(skillX, skillY, contentW - 40, skillH, 
                          isSelected ? PIPBOY_GREEN : PIPBOY_DIM);
        
        // Skill name and level
        DrawText(skill.name.c_str(), skillX + 10, skillY + 8, 20, textColor);
        DrawText(TextFormat("Level %d / %d", skill.currentLevel, skill.maxLevel), 
                 skillX + 10, skillY + 32, 16, PIPBOY_DIM);
        
        // Description
        DrawText(skill.description.c_str(), skillX + 10, skillY + 52, 14, PIPBOY_DIM);
        
        // Level bars
        int levelBarX = skillX + contentW - 200;
        int levelBarY = skillY + 20;
        int levelBarW = 150;
        int levelBarH = 10;
        
        for (int lvl = 0; lvl < skill.maxLevel; lvl++) {
            Color barColor = lvl < skill.currentLevel ? PIPBOY_GREEN : Color{40, 60, 40, 255};
            DrawRectangle(levelBarX + lvl * 32, levelBarY, 28, levelBarH, barColor);
            DrawRectangleLines(levelBarX + lvl * 32, levelBarY, 28, levelBarH, PIPBOY_DIM);
        }
        
        // Cost to upgrade
        if (skill.currentLevel < skill.maxLevel) {
            DrawText(TextFormat("Cost: %d SP", skill.cost), 
                     levelBarX, levelBarY + 15, 14, Color{255, 215, 0, 255});
        } else {
            DrawText("MAX", levelBarX + 50, levelBarY + 15, 14, PIPBOY_GREEN);
        }
        
        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= skillX && mousePos.x <= skillX + contentW - 40 &&
            mousePos.y >= skillY && mousePos.y <= skillY + skillH) {
            skillSelection = (int)i;
        }
        
        skillY += skillH + 10;
    }
    
    // Instructions
    DrawText("Press SPACE or A to upgrade selected skill", 
             contentX + 20, contentY + contentH - 40, 16, PIPBOY_DIM);
    
    // Upgrade action
    bool upgradePressed = IsKeyPressed(KEY_SPACE) || 
                         (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
    
    if (upgradePressed && skillSelection >= 0 && skillSelection < (int)skills.size()) {
        const Skill& skill = skills[skillSelection];
        if (skill.currentLevel < skill.maxLevel && 
            g_PlayerProgression.GetSkillPoints() >= skill.cost) {
            g_SkillTree.UpgradeSkill(skill.type, g_PlayerProgression);
        } else {
            TraceLog(LOG_INFO, "Cannot upgrade skill - insufficient points or max level reached");
        }
    }
}

// Draw quests screen
void DrawQuestsScreen(int screenW, int screenH, int contentX, int contentY, int contentW, int contentH, bool useController) {
    // Header
    DrawText("ACTIVE QUESTS", contentX + 20, contentY + 10, 24, PIPBOY_GREEN);
    
    auto activeQuests = g_QuestManager.GetActiveQuests();
    
    if (activeQuests.empty()) {
        DrawText("No active quests", contentX + contentW / 2 - 100, 
                 contentY + contentH / 2, 20, PIPBOY_DIM);
        DrawText("Complete objectives to gain XP and level up!", 
                 contentX + contentW / 2 - 200, contentY + contentH / 2 + 30, 16, PIPBOY_DIM);
        return;
    }
    
    // Navigation
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        questSelection = (questSelection - 1 + (int)activeQuests.size()) % (int)activeQuests.size();
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        questSelection = (questSelection + 1) % (int)activeQuests.size();
    }
    
    int questY = contentY + 50;
    
    for (size_t i = 0; i < activeQuests.size(); i++) {
        Quest* quest = activeQuests[i];
        
        bool isSelected = (questSelection == (int)i);
        Color bgColor = isSelected ? PIPBOY_SELECTED : PIPBOY_DARK;
        
        // Calculate quest box height based on objectives
        int questH = 60 + (int)quest->objectives.size() * 25 + 30;
        int questX = contentX + 20;
        int questW = contentW - 40;
        
        DrawRectangle(questX, questY, questW, questH, bgColor);
        DrawRectangleLines(questX, questY, questW, questH, 
                          isSelected ? PIPBOY_GREEN : PIPBOY_DIM);
        
        // Quest name
        DrawText(quest->name.c_str(), questX + 15, questY + 10, 20, PIPBOY_GREEN);
        
        // Description
        DrawText(quest->description.c_str(), questX + 15, questY + 35, 14, PIPBOY_DIM);
        
        // XP reward
        DrawText(TextFormat("Reward: %d XP", quest->xpReward), 
                 questX + questW - 150, questY + 10, 16, Color{255, 215, 0, 255});
        
        // Objectives
        int objY = questY + 60;
        DrawText("Objectives:", questX + 15, objY, 16, PIPBOY_GREEN);
        objY += 25;
        
        bool allComplete = true;
        for (const auto& obj : quest->objectives) {
            bool objComplete = obj.currentCount >= obj.targetCount;
            if (!objComplete) allComplete = false;
            
            Color objColor = objComplete ? Color{50, 255, 50, 255} : PIPBOY_DIM;
            const char* checkbox = objComplete ? "[X]" : "[ ]";
            
            DrawText(TextFormat("%s %s [%d/%d]", checkbox, obj.description.c_str(), 
                     obj.currentCount, obj.targetCount),
                     questX + 25, objY, 14, objColor);
            
            // Progress bar
            float progress = fminf(1.0f, (float)obj.currentCount / (float)obj.targetCount);
            int barW = 200;
            int barH = 8;
            int barX = questX + questW - barW - 20;
            
            DrawRectangle(barX, objY + 3, barW, barH, PIPBOY_DARK);
            DrawRectangle(barX, objY + 3, (int)(barW * progress), barH, objColor);
            DrawRectangleLines(barX, objY + 3, barW, barH, PIPBOY_DIM);
            
            objY += 25;
        }
        
        // Complete button (if all objectives done)
        if (allComplete && !quest->isCompleted) {
            int btnX = questX + questW / 2 - 100;
            int btnY = questY + questH - 35;
            int btnW = 200;
            int btnH = 30;
            
            DrawRectangle(btnX, btnY, btnW, btnH, PIPBOY_GREEN);
            DrawText("COMPLETE QUEST (E)", btnX + 20, btnY + 8, 16, BLACK);
            
            bool completePressed = IsKeyPressed(KEY_E) || 
                                  (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
            
            if (completePressed && isSelected) {
                g_QuestManager.CompleteQuest(quest->id, g_PlayerProgression);
            }
        }
        
        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= questX && mousePos.x <= questX + questW &&
            mousePos.y >= questY && mousePos.y <= questY + questH) {
            questSelection = (int)i;
        }
        
        questY += questH + 15;
    }
    
    // Instructions
    DrawText("Press E or A to complete selected quest when objectives are done", 
             contentX + 20, contentY + contentH - 40, 14, PIPBOY_DIM);
}