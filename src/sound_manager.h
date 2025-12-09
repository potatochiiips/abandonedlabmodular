#pragma once
#include "globals.h"
#include <map>

// Sound effect IDs
enum SoundID {
    SND_PISTOL_SHOT,
    SND_RIFLE_SHOT,
    SND_RELOAD,
    SND_EMPTY_CLICK,
    SND_ITEM_PICKUP,
    SND_ITEM_USE,
    SND_FOOTSTEP_CONCRETE,
    SND_FOOTSTEP_GRASS,
    SND_FOOTSTEP_METAL,
    SND_DOOR_OPEN,
    SND_DOOR_CLOSE,
    SND_FLASHLIGHT_TOGGLE,
    SND_UI_SELECT,
    SND_UI_BACK,
    SND_UI_ERROR,
    SND_LEVEL_UP,
    SND_QUEST_COMPLETE,
    SND_CRAFT_SUCCESS,
    SND_LOW_HEALTH_HEARTBEAT,
    SND_BREATHING_HEAVY,
    SND_JUMP,
    SND_LAND,
    SND_COUNT
};

// Music track IDs
enum MusicID {
    MUS_MENU,
    MUS_AMBIENT_OUTSIDE,
    MUS_AMBIENT_INSIDE,
    MUS_EXPLORATION,
    MUS_TENSION,
    MUS_COUNT
};

// Sound manager class
class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    // Initialize audio device and load sounds
    void Initialize();

    // Play a sound effect
    void PlaySound(SoundID id, float volume = 1.0f);

    // Play sound at 3D position (with distance attenuation)
    void PlaySound3D(SoundID id, Vector3 position, Vector3 listenerPos, float maxDistance = 50.0f, float volume = 1.0f);

    // Play music track (fades out previous track)
    void PlayMusic(MusicID id, bool loop = true, float fadeTime = 2.0f);

    // Stop music
    void StopMusic(float fadeTime = 2.0f);

    // Update music (handles fading)
    void Update(float deltaTime);

    // Set master volumes
    void SetMasterVolume(float volume);
    void SetSFXVolume(float volume);
    void SetMusicVolume(float volume);

    // Get volume levels
    float GetMasterVolume() const { return masterVolume; }
    float GetSFXVolume() const { return sfxVolume; }
    float GetMusicVolume() const { return musicVolume; }

    // Check if sound is loaded
    bool IsSoundLoaded(SoundID id);
    bool IsMusicLoaded(MusicID id);

    // Reload all sounds
    void Reload();

    // Cleanup
    void Unload();

    // Save/Load audio settings
    void SaveAudioSettings();
    void LoadAudioSettings();

private:
    std::map<SoundID, Sound> sounds;
    std::map<SoundID, bool> soundsLoaded;
    std::map<MusicID, Music> musicTracks;
    std::map<MusicID, bool> musicLoaded;

    MusicID currentMusicTrack;
    MusicID nextMusicTrack;
    bool isMusicPlaying;
    bool isFadingOut;
    bool isFadingIn;
    float currentMusicVolume;
    float targetMusicVolume;
    float musicFadeSpeed;

    float masterVolume;
    float sfxVolume;
    float musicVolume;

    // Load individual sound with error handling
    bool LoadSoundFile(SoundID id, const char* filename);
    bool LoadMusicFile(MusicID id, const char* filename);

    // Generate procedural sound as fallback
    Sound CreateProceduralSound(SoundID id);
};

// Global sound manager instance
extern SoundManager* g_SoundManager;

// Initialize sound system
void InitializeSoundSystem();

// Cleanup sound system
void CleanupSoundSystem();