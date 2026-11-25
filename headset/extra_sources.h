// Readers for the local panel additions (redux panel): GPU memory and the audio devices.
#pragma once

#include <string>

/** What the audio card shows: the default output and input. */
struct AudioDevices {
    bool valid = false;      ///< pactl answered
    std::string outName;     ///< shortened description ("INZONE Buds")
    double outVolPct = -1;   ///< average of the channels; -1 = unknown
    bool outMuted = false;
    bool outHeadphones = false;  ///< not the headset's own speakers
    std::string inName;
    double inVolPct = -1;
    bool inMuted = false;
};

/**
 * Memory the GPU holds right now (resident GEM objects in debugfs dri/N/gem), in GiB.
 * @return GiB, or NaN when the file can't be read
 */
double readGpuMemGib();

/**
 * Asks PipeWire (through pactl) for the default sink and source. Spawns pactl, so it's meant for a 2 s source.
 * @return the devices (valid = false if pactl failed)
 */
AudioDevices readAudioDevices();

/**
 * Shortens a device description for the panel: "INZONE Buds Analog Stereo" → "INZONE Buds",
 * "Built-in Audio Playback" → "Built-in".
 */
std::string shortDeviceName(std::string name);
