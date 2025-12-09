#include "sound_manager.h"

// Global instance
SoundManager* g_SoundManager = nullptr;

// Sound file paths
static const char* SOUND_PATHS[SND_COUNT] = {
    "assets/sounds/pistol_shot.wav",
    "assets/sounds/rifle_shot.wav",
    "assets/sounds/reload.wav",
    "assets/sounds/empty_click.wav",
    "assets/sounds/item_pickup.wav",
    "assets/sounds/item_use.wav",
    "assets/sounds/footstep_concrete.wav",
    "assets/sounds/footstep_grass.wav",
    "assets/sounds/footstep_metal.wav",
    "assets/sounds/door_open.wav",
    "assets/sounds/door_close.wav",
    "assets/sounds/flashlight_toggle.wav",
    "assets/sounds/ui_select.wav",
    "assets/sounds/ui_back.wav",
    "assets/sounds/ui_error.wav",
    "assets/sounds/level_up.wav",
    "assets/sounds/quest_complete.wav",
    "assets/sounds/craft_success.wav",
    "assets/sounds/heartbeat.wav",
    "assets/sounds/breathing_heavy.wav",
    "assets/sounds/jump.wav",
    "assets/sounds/land.wav"
};

// Music file paths
static const char* MUSIC_PATHS[MUS_COUNT] = {
    "assets/music/menu_theme.ogg",
    "assets/music/ambient_outside.ogg",
    "assets/music/ambient_inside.ogg",
    "assets/music/exploration.ogg",
    "assets/music/tension.ogg"
};

SoundManager::SoundManager() {
    currentMusicTrack = MUS_MENU;
    nextMusicTrack = MUS_MENU;
    isMusicPlaying = false;
    isFadingOut = false;
    isFadingIn = false;
    currentMusicVolume = 0.0f;
    targetMusicVolume = 0.0f;
    musicFadeSpeed = 0.5f;
    masterVolume = 1.0f;
    sfxVolume = 1.0f;
    musicVolume = 0.6f;
}

SoundManager::~SoundManager() {
    Unload();
}

void SoundManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Sound Manager...");

    InitAudioDevice();

    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_ERROR, "Failed to initialize audio device!");
        return;
    }

    // Load all sound effects
    for (int i = 0; i < SND_COUNT; i++) {
        SoundID id = (SoundID)i;

        if (!LoadSoundFile(id, SOUND_PATHS[i])) {
            TraceLog(LOG_WARNING, "Failed to load sound: %s - Using procedural fallback", SOUND_PATHS[i]);
            sounds[id] = CreateProceduralSound(id);
            soundsLoaded[id] = true;
        }
    }

    // Load all music tracks
    for (int i = 0; i < MUS_COUNT; i++) {
        MusicID id = (MusicID)i;

        if (LoadMusicFile(id, MUSIC_PATHS[i])) {
            TraceLog(LOG_INFO, "Loaded music: %s", MUSIC_PATHS[i]);
        }
        else {
            TraceLog(LOG_WARNING, "Failed to load music: %s", MUSIC_PATHS[i]);
        }
    }

    TraceLog(LOG_INFO, "Sound Manager initialized.");
}

bool SoundManager::LoadSoundFile(SoundID id, const char* filename) {
    if (FileExists(filename)) {
        Sound snd = LoadSound(filename);
        if (snd.frameCount > 0) {
            sounds[id] = snd;
            soundsLoaded[id] = true;
            TraceLog(LOG_INFO, "Loaded sound: %s", filename);
            return true;
        }
    }

    soundsLoaded[id] = false;
    return false;
}

bool SoundManager::LoadMusicFile(MusicID id, const char* filename) {
    if (FileExists(filename)) {
        Music mus = LoadMusicStream(filename);
        if (mus.frameCount > 0) {
            musicTracks[id] = mus;
            musicLoaded[id] = true;
            return true;
        }
    }

    musicLoaded[id] = false;
    return false;
}

Sound SoundManager::CreateProceduralSound(SoundID id) {
    // Create simple procedural sounds as fallbacks using LoadWave
    Wave wave;

    switch (id) {
    case SND_PISTOL_SHOT:
    case SND_RIFLE_SHOT:
        // Sharp bang sound - use noise
        wave = LoadWaveFromMemory(".wav", (unsigned char*)"", 0);
        wave = GenImageWhiteNoise(256, 1).data ? LoadWaveFromMemory(".wav", (unsigned char*)"", 0) : LoadWave("");
        break;

    case SND_RELOAD:
    case SND_EMPTY_CLICK:
    case SND_ITEM_PICKUP:
    case SND_ITEM_USE:
    case SND_FOOTSTEP_CONCRETE:
    case SND_FOOTSTEP_GRASS:
    case SND_FOOTSTEP_METAL:
    case SND_DOOR_OPEN:
    case SND_DOOR_CLOSE:
    case SND_FLASHLIGHT_TOGGLE:
    case SND_UI_SELECT:
    case SND_UI_BACK:
    case SND_UI_ERROR:
    case SND_LEVEL_UP:
    case SND_QUEST_COMPLETE:
    case SND_CRAFT_SUCCESS:
    case SND_LOW_HEALTH_HEARTBEAT:
    case SND_BREATHING_HEAVY:
    case SND_JUMP:
    case SND_LAND:
    default:
        // Create a silent wave as fallback
        wave.frameCount = 4410; // 0.1 seconds at 44100 Hz
        wave.sampleRate = 44100;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = (short*)MemAlloc(wave.frameCount * sizeof(short));
        memset(wave.data, 0, wave.frameCount * sizeof(short));
        break;
    }

    Sound snd = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return snd;
}

void SoundManager::PlaySound(SoundID id, float volume) {
    if (sounds.find(id) != sounds.end() && soundsLoaded[id]) {
        SetSoundVolume(sounds[id], volume * sfxVolume * masterVolume);
        ::PlaySound(sounds[id]); // Use :: to specify global PlaySound
    }
}

void SoundManager::PlaySound3D(SoundID id, Vector3 position, Vector3 listenerPos, float maxDistance, float volume) {
    if (sounds.find(id) == sounds.end() || !soundsLoaded[id]) return;

    float distance = Vector3Distance(position, listenerPos);

    if (distance > maxDistance) return;

    // Calculate attenuation
    float attenuation = 1.0f - (distance / maxDistance);
    attenuation = Clamp(attenuation, 0.0f, 1.0f);

    SetSoundVolume(sounds[id], volume * attenuation * sfxVolume * masterVolume);
    ::PlaySound(sounds[id]); // Use :: to specify global PlaySound
}

void SoundManager::PlayMusic(MusicID id, bool loop, float fadeTime) {
    if (musicTracks.find(id) == musicTracks.end() || !musicLoaded[id]) return;

    // Fade out current music if playing
    if (isMusicPlaying && currentMusicTrack != id) {
        targetMusicVolume = 0.0f;
        musicFadeSpeed = 1.0f / fadeTime;
        nextMusicTrack = id;
        isFadingOut = true;
    }
    else {
        // Start new track
        if (currentMusicTrack != id || !isMusicPlaying) {
            if (isMusicPlaying) {
                StopMusicStream(musicTracks[currentMusicTrack]);
            }

            currentMusicTrack = id;
            musicTracks[id].looping = loop;
            PlayMusicStream(musicTracks[id]);
            isMusicPlaying = true;
            currentMusicVolume = 0.0f;
            targetMusicVolume = musicVolume * masterVolume;
            musicFadeSpeed = 1.0f / fadeTime;
            isFadingIn = true;
            isFadingOut = false;
        }
    }
}

void SoundManager::StopMusic(float fadeTime) {
    if (isMusicPlaying) {
        targetMusicVolume = 0.0f;
        musicFadeSpeed = 1.0f / fadeTime;
        isFadingOut = true;
    }
}

void SoundManager::Update(float deltaTime) {
    if (!isMusicPlaying) return;

    // Update music stream
    UpdateMusicStream(musicTracks[currentMusicTrack]);

    // Handle fading
    if (currentMusicVolume != targetMusicVolume) {
        if (currentMusicVolume < targetMusicVolume) {
            currentMusicVolume += musicFadeSpeed * deltaTime;
            if (currentMusicVolume > targetMusicVolume) {
                currentMusicVolume = targetMusicVolume;
                isFadingIn = false;
            }
        }
        else {
            currentMusicVolume -= musicFadeSpeed * deltaTime;
            if (currentMusicVolume < targetMusicVolume) {
                currentMusicVolume = targetMusicVolume;
            }
        }

        SetMusicVolume(musicTracks[currentMusicTrack], currentMusicVolume);

        // Stop music if faded out
        if (currentMusicVolume <= 0.0f && targetMusicVolume <= 0.0f) {
            StopMusicStream(musicTracks[currentMusicTrack]);
            isMusicPlaying = false;
            isFadingOut = false;

            // Start next track if queued
            if (nextMusicTrack != currentMusicTrack) {
                PlayMusic(nextMusicTrack, true, 2.0f);
            }
        }
    }
}

void SoundManager::SetMasterVolume(float volume) {
    masterVolume = Clamp(volume, 0.0f, 1.0f);

    // Update current music volume
    if (isMusicPlaying) {
        SetMusicVolume(musicTracks[currentMusicTrack], musicVolume * masterVolume);
    }
}

void SoundManager::SetSFXVolume(float volume) {
    sfxVolume = Clamp(volume, 0.0f, 1.0f);
}

void SoundManager::SetMusicVolume(float volume) {
    musicVolume = Clamp(volume, 0.0f, 1.0f);

    // Update current music volume
    if (isMusicPlaying) {
        targetMusicVolume = musicVolume * masterVolume;
    }
}

bool SoundManager::IsSoundLoaded(SoundID id) {
    return soundsLoaded[id];
}

bool SoundManager::IsMusicLoaded(MusicID id) {
    return musicLoaded[id];
}

void SoundManager::Reload() {
    Unload();
    Initialize();
}

void SoundManager::Unload() {
    // Stop and unload music
    for (auto& pair : musicTracks) {
        if (musicLoaded[pair.first]) {
            StopMusicStream(pair.second);
            UnloadMusicStream(pair.second);
        }
    }
    musicTracks.clear();
    musicLoaded.clear();

    // Unload sounds
    for (auto& pair : sounds) {
        if (soundsLoaded[pair.first]) {
            UnloadSound(pair.second);
        }
    }
    sounds.clear();
    soundsLoaded.clear();

    CloseAudioDevice();
}

// Global initialization
void InitializeSoundSystem() {
    g_SoundManager = new SoundManager();
    g_SoundManager->Initialize();
    TraceLog(LOG_INFO, "Sound system initialized");
}

void CleanupSoundSystem() {
    if (g_SoundManager) {
        delete g_SoundManager;
        g_SoundManager = nullptr;
    }
    TraceLog(LOG_INFO, "Sound system cleaned up");
}