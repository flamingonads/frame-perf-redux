# frame-perf-redux changelog

Changes this fork makes on top of [frame-perf-overlay](https://github.com/sasaken1102r/frame-perf-overlay). The original's own history is in [CHANGELOG.md](CHANGELOG.md) (Japanese).

## 0.4.1 (2026-11-27)

First release of the fork, based on the original's 0.4.0 plus its unreleased changes up to commit 066a5c8.

### Install and updates

- New installer for the fork: `install-redux.sh` (see [Install this fork](README.md#install-this-fork)). It downloads the latest release, checks it against `SHA256SUMS`, checks the archive, and runs the release's `install.sh`. `uninstall` removes the app.
- The **Update** button in the Perf tab now checks this fork's releases instead of the original's.

### Background graphs

- While the headset is worn and the panel is hidden or faded, the frame-time graphs keep recording, so the last 60 seconds are there when you look. Only the frame timing is read and nothing is drawn. Measured cost: the same as off, within measuring error. `"background_graphs": false` in `config.json` turns it off. Taking the headset off (standby) still stops everything.

### Panel

- One column of four cards (Overview, Memory, Wi-Fi/Audio, Batteries) instead of the two-column grid.
- Overview: fps against the refresh rate, session reprojected % and dropped frames, clock, game title and play time (long titles scroll), CPU and GPU load with temperature, and filled 60-second frame-time graphs with the frame budget.
- Memory: RAM use with the GPU share split out.
- Wi-Fi/Audio: signal, speeds, output device and microphone, and whether the microphone is live or muted (audio devices read every 5 s).
- Batteries: power draw, USB-C charger state (with a steadier warning near full), headset battery with time remaining or to full, and both controller batteries.

### Center marker

- An optional white ring on the floor at the center of the playspace (40 cm, 10 % opacity), with a Center marker On/Off toggle in the settings footer (`floor_ring` in `config.json`).
