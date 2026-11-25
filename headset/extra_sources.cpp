#include "extra_sources.h"

#include "json.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

namespace {

bool endsWith(const std::string& s, const std::string& tail) {
    return s.size() >= tail.size() && s.compare(s.size() - tail.size(), tail.size(), tail) == 0;
}

/** Average of the channels' "value_percent" ("76%"). */
double volumePct(const JsonValue* volume) {
    if (volume == nullptr || !volume->isObject()) return -1;
    double sum = 0;
    int n = 0;
    for (const auto& member : volume->members) {
        const JsonValue* pct = member.second.get("value_percent");
        if (pct == nullptr || !pct->isString()) continue;
        sum += std::atof(pct->text.c_str());
        ++n;
    }
    return n > 0 ? sum / n : -1;
}

/** The entry of a `pactl -f json list` array with this name. */
const JsonValue* findByName(const JsonValue& list, const std::string& name) {
    if (list.type != JsonValue::Type::Array) return nullptr;
    for (const JsonValue& item : list.items) {
        const JsonValue* n = item.get("name");
        if (n != nullptr && n->isString() && n->text == name) return &item;
    }
    return nullptr;
}

bool parseLine(const std::string& line, JsonValue& out) {
    std::string error;
    return !line.empty() && parseJson(line, out, error);
}

}  // namespace

std::string shortDeviceName(std::string name) {
    static const char* const kTails[] = {" Analog Stereo", " Digital Stereo", " Stereo", " Mono", " Playback",
                                         " Capture", " Audio", " Headset", " Hands-Free"};
    for (bool again = true; again;) {
        again = false;
        for (const char* tail : kTails) {
            if (endsWith(name, tail) && name.size() > std::strlen(tail)) {
                name.resize(name.size() - std::strlen(tail));
                again = true;
            }
        }
    }
    return name;
}

double readGpuMemGib() {
    for (const char* path : {"/sys/kernel/debug/dri/0/gem", "/sys/kernel/debug/dri/128/gem"}) {
        std::ifstream file(path);
        if (!file) continue;
        std::string line;
        double bytes = NAN;
        while (std::getline(file, line)) {
            // "Resident:  1602 objects, 1535946752 bytes" (near the end)
            if (line.compare(0, 9, "Resident:") != 0) continue;
            const size_t comma = line.find(',');
            if (comma != std::string::npos) bytes = std::atof(line.c_str() + comma + 1);
        }
        if (!std::isnan(bytes)) return bytes / (1024.0 * 1024.0 * 1024.0);
    }
    return NAN;
}

AudioDevices readAudioDevices() {
    AudioDevices d;
    // One shell, three answers, one JSON document per line
    FILE* pipe = popen("pactl -f json info 2>/dev/null; echo; pactl -f json list sinks 2>/dev/null; echo; "
                       "pactl -f json list sources 2>/dev/null; echo",
                       "r");
    if (pipe == nullptr) return d;
    std::string all;
    char buffer[8192];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), pipe)) > 0) all.append(buffer, n);
    pclose(pipe);
    std::istringstream lines(all);
    std::string infoLine, sinksLine, sourcesLine;
    std::getline(lines, infoLine);
    std::getline(lines, sinksLine);
    std::getline(lines, sourcesLine);
    JsonValue info, sinks, sources;
    if (!parseLine(infoLine, info) || !parseLine(sinksLine, sinks) || !parseLine(sourcesLine, sources)) return d;
    const JsonValue* sinkName = info.get("default_sink_name");
    const JsonValue* sourceName = info.get("default_source_name");
    if (sinkName != nullptr && sinkName->isString()) {
        if (const JsonValue* sink = findByName(sinks, sinkName->text)) {
            const JsonValue* desc = sink->get("description");
            d.outName = shortDeviceName(desc != nullptr && desc->isString() ? desc->text : sinkName->text);
            d.outVolPct = volumePct(sink->get("volume"));
            const JsonValue* mute = sink->get("mute");
            d.outMuted = mute != nullptr && mute->isBool() && mute->boolean;
            d.outHeadphones = sinkName->text.find("platform-sound") == std::string::npos;
            if (!d.outHeadphones) d.outName = "Speaker";  // the headset's own speakers
        }
    }
    if (sourceName != nullptr && sourceName->isString()) {
        if (const JsonValue* source = findByName(sources, sourceName->text)) {
            const JsonValue* desc = source->get("description");
            d.inName = shortDeviceName(desc != nullptr && desc->isString() ? desc->text : sourceName->text);
            d.inVolPct = volumePct(source->get("volume"));
            const JsonValue* mute = source->get("mute");
            d.inMuted = mute != nullptr && mute->isBool() && mute->boolean;
        }
    }
    d.valid = !d.outName.empty() || !d.inName.empty();
    return d;
}
