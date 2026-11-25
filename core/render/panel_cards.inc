// redux panel: four cards (Overview, Memory, Wi-Fi & audio, Power & batteries).
// Included inside widgets.cpp's anonymous namespace, so it uses the same helpers and colours.
// Kept in its own file so upstream changes to widgets.cpp merge cleanly.

// ---- Look ----
constexpr Color kTile = hexColor(0x11161d);       // inner tiles, between the panel's ground and the card
constexpr Color kToneGreen = hexColor(0x2d723b);  // battery-bar tones, used for every green / yellow / red fill
constexpr Color kToneYellow = hexColor(0x7d6022);
constexpr Color kToneRed = hexColor(0x923938);
constexpr double kPad = 8;                         // card edge to tile
constexpr double kTileGap = 8;

// Heights
constexpr double kOvTopH = 92;    // fps tile and clock tile
constexpr double kOvStripH = 84;  // CPU and GPU strips
constexpr int kOverviewH = static_cast<int>(kPad + kOvTopH + kTileGap + kOvStripH + kTileGap + kOvStripH + kPad);
constexpr int kMemoryH = 104;
constexpr int kLinkAudioH = static_cast<int>(kPad + 84 + kPad);
constexpr int kPowerBatteryH = 124;

// RAM warning (no setting for it upstream)
constexpr double kRamWarnPct = 85.0;
constexpr double kRamCritPct = 95.0;

Color toneOf(Level level) {
    return level == Level::Crit ? kToneRed : level == Level::Warn ? kToneYellow : kToneGreen;
}

void fillTile(const Pen& pen, double x, double y, double w, double h) {
    pen.color(kTile);
    pen.roundedRect(x, y, w, h, 8);
    cairo_fill(pen.cr);
}

/** A filled pill with light text. @return its width */
double drawTonePill(const Pen& pen, double x, double top, const std::string& s, Color fill, bool alignRight = false,
                    double size = 13) {
    const double w = pen.measure(s, size, true) + 16;
    const double left = alignRight ? x - w : x;
    pen.color(fill);
    pen.roundedRect(left, top, w, size + 8, (size + 8) / 2);
    cairo_fill(pen.cr);
    pen.text(left + 8, top + size + 2, s, size, kText, true);
    return w;
}

/** "2h 40m", "35m" */
std::string shortDuration(double seconds) {
    if (std::isnan(seconds) || seconds < 0) return "";
    const long m = static_cast<long>(seconds / 60.0 + 0.5);
    char b[32];
    if (m >= 60) std::snprintf(b, sizeof(b), "%ldh %02ldm", m / 60, m % 60);
    else std::snprintf(b, sizeof(b), "%ldm", m);
    return b;
}

/** A frame-time line coloured per segment: normal colour, yellow past warn, red past crit. */
void drawLevelSeries(const Pen& pen, const GraphArea& area, const History& history, Color c, double warn,
                     double crit) {
    cairo_save(pen.cr);
    pen.roundedRect(area.x, area.y, area.w, area.h, 6);
    cairo_clip(pen.cr);
    cairo_set_line_width(pen.cr, 2.0);
    cairo_set_line_cap(pen.cr, CAIRO_LINE_CAP_ROUND);
    bool have = false;
    double px = 0, py = 0, pv = 0;
    for (const auto& p : history.points()) {
        if (std::isnan(p.v)) {
            have = false;
            continue;
        }
        const double x = area.xOf(p.t), y = area.yOf(p.v);
        if (have) {
            const double v = std::max(pv, p.v);
            pen.color(v > crit ? kDanger : v > warn ? kWarn : c);
            cairo_move_to(pen.cr, px, py);
            cairo_line_to(pen.cr, x, y);
            cairo_stroke(pen.cr);
        }
        px = x;
        py = y;
        pv = p.v;
        have = true;
    }
    cairo_restore(pen.cr);
}

// ---- Icons (line drawings, about 16-22 px) ----

void iconHeadset(const Pen& pen, double x, double cy, Color c) {
    cairo_save(pen.cr);
    cairo_new_path(pen.cr);
    pen.color(c);
    cairo_set_line_width(pen.cr, 1.6);
    cairo_set_line_join(pen.cr, CAIRO_LINE_JOIN_ROUND);
    const double t = cy - 6, b = cy + 6;
    cairo_move_to(pen.cr, x + 3, t);
    cairo_line_to(pen.cr, x + 19, t);
    cairo_curve_to(pen.cr, x + 21, t, x + 21, t, x + 21, t + 2);
    cairo_line_to(pen.cr, x + 21, b - 3);
    cairo_curve_to(pen.cr, x + 21, b, x + 21, b, x + 18, b);
    cairo_line_to(pen.cr, x + 15, b);
    cairo_line_to(pen.cr, x + 13, b - 3);
    cairo_line_to(pen.cr, x + 9, b - 3);
    cairo_line_to(pen.cr, x + 7, b);
    cairo_line_to(pen.cr, x + 4, b);
    cairo_curve_to(pen.cr, x + 1, b, x + 1, b, x + 1, b - 3);
    cairo_line_to(pen.cr, x + 1, t + 2);
    cairo_curve_to(pen.cr, x + 1, t, x + 1, t, x + 3, t);
    cairo_close_path(pen.cr);
    cairo_stroke(pen.cr);
    cairo_restore(pen.cr);
}

void iconController(const Pen& pen, double x, double cy, Color c) {
    cairo_save(pen.cr);
    pen.color(c);
    cairo_set_line_width(pen.cr, 1.6);
    cairo_new_path(pen.cr);
    cairo_arc(pen.cr, x + 7, cy - 4, 4.2, 0, 2 * M_PI);
    cairo_stroke(pen.cr);
    cairo_move_to(pen.cr, x + 5, cy);
    cairo_line_to(pen.cr, x + 5.5, cy + 8);
    cairo_line_to(pen.cr, x + 8.5, cy + 8);
    cairo_line_to(pen.cr, x + 9, cy);
    cairo_stroke(pen.cr);
    cairo_restore(pen.cr);
}

void iconPlug(const Pen& pen, double x, double cy, Color c, bool crossed) {
    cairo_save(pen.cr);
    pen.color(c);
    const double t = cy - 8;
    cairo_rectangle(pen.cr, x + 4, t, 2, 5);
    cairo_rectangle(pen.cr, x + 10, t, 2, 5);
    cairo_fill(pen.cr);
    cairo_move_to(pen.cr, x + 2, t + 5);
    cairo_line_to(pen.cr, x + 14, t + 5);
    cairo_line_to(pen.cr, x + 14, t + 8);
    cairo_arc(pen.cr, x + 8, t + 8, 6, 0, M_PI);
    cairo_close_path(pen.cr);
    cairo_fill(pen.cr);
    cairo_rectangle(pen.cr, x + 7, t + 13, 2, 4);
    cairo_fill(pen.cr);
    if (crossed) {
        cairo_set_line_width(pen.cr, 1.6);
        cairo_move_to(pen.cr, x + 1, cy + 9);
        cairo_line_to(pen.cr, x + 16, cy - 8);
        cairo_stroke(pen.cr);
    }
    cairo_restore(pen.cr);
}

/** Antenna mast + 4 bars. bars 0 = not connected (crossed). */
void iconAntenna(const Pen& pen, double x, double baseline, int bars, Color c) {
    cairo_save(pen.cr);
    cairo_set_line_width(pen.cr, 1.8);
    cairo_set_line_cap(pen.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(pen.cr, CAIRO_LINE_JOIN_ROUND);
    pen.color(bars > 0 ? c : kTextMuted);
    cairo_move_to(pen.cr, x + 5, baseline);
    cairo_line_to(pen.cr, x + 5, baseline - 16);
    cairo_move_to(pen.cr, x + 1, baseline - 20);
    cairo_line_to(pen.cr, x + 5, baseline - 15);
    cairo_line_to(pen.cr, x + 9, baseline - 20);
    cairo_stroke(pen.cr);
    for (int k = 0; k < 4; ++k) {
        const double h = 5 + k * 4;
        if (k < bars) pen.color(c); else pen.color(kControlHover);
        pen.roundedRect(x + 14 + k * 6, baseline - h, 4, h, 1);
        cairo_fill(pen.cr);
    }
    if (bars == 0) {
        pen.color(kTextMuted);
        cairo_move_to(pen.cr, x + 2, baseline);
        cairo_line_to(pen.cr, x + 36, baseline - 20);
        cairo_stroke(pen.cr);
    }
    cairo_restore(pen.cr);
}

void iconHeadphones(const Pen& pen, double x, double cy, Color c) {
    cairo_save(pen.cr);
    cairo_new_path(pen.cr);
    pen.color(c);
    cairo_set_line_width(pen.cr, 1.6);
    cairo_arc(pen.cr, x + 8, cy + 1, 5.6, M_PI, 2 * M_PI);
    cairo_stroke(pen.cr);
    pen.roundedRect(x + 1.5, cy, 3.6, 6.5, 1.2);
    cairo_fill(pen.cr);
    pen.roundedRect(x + 10.9, cy, 3.6, 6.5, 1.2);
    cairo_fill(pen.cr);
    cairo_restore(pen.cr);
}

void iconSpeaker(const Pen& pen, double x, double cy, Color c) {
    cairo_save(pen.cr);
    pen.color(c);
    cairo_move_to(pen.cr, x + 1, cy - 2.5);
    cairo_line_to(pen.cr, x + 4, cy - 2.5);
    cairo_line_to(pen.cr, x + 8, cy - 6);
    cairo_line_to(pen.cr, x + 8, cy + 6);
    cairo_line_to(pen.cr, x + 4, cy + 2.5);
    cairo_line_to(pen.cr, x + 1, cy + 2.5);
    cairo_close_path(pen.cr);
    cairo_fill(pen.cr);
    cairo_set_line_width(pen.cr, 1.4);
    cairo_new_path(pen.cr);
    cairo_arc(pen.cr, x + 8, cy, 4, -M_PI / 4, M_PI / 4);
    cairo_stroke(pen.cr);
    cairo_new_path(pen.cr);
    cairo_arc(pen.cr, x + 8, cy, 7, -M_PI / 4, M_PI / 4);
    cairo_stroke(pen.cr);
    cairo_restore(pen.cr);
}

void iconMic(const Pen& pen, double x, double cy, bool muted) {
    const Color c = muted ? kDanger : kText;
    cairo_save(pen.cr);
    pen.color(c);
    pen.roundedRect(x + 5.5, cy - 7, 5, 9, 2.5);
    cairo_fill(pen.cr);
    cairo_set_line_width(pen.cr, 1.4);
    cairo_new_path(pen.cr);
    cairo_arc_negative(pen.cr, x + 8, cy - 0.5, 5, M_PI, 0);
    cairo_stroke(pen.cr);
    cairo_move_to(pen.cr, x + 8, cy + 4.5);
    cairo_line_to(pen.cr, x + 8, cy + 7);
    cairo_stroke(pen.cr);
    if (muted) {
        cairo_set_line_width(pen.cr, 1.8);
        cairo_move_to(pen.cr, x + 2, cy - 7.5);
        cairo_line_to(pen.cr, x + 14, cy + 6.5);
        cairo_stroke(pen.cr);
    }
    cairo_restore(pen.cr);
}

/** Battery with the % inside, filled in its tone. NaN = not connected ("--"). @return width */
double drawToneBattery(const Pen& pen, double x, double cy, double pct, Level level, bool charging, double w) {
    const double h = 22, top = cy - h / 2;
    cairo_save(pen.cr);
    pen.color(kBorder);
    cairo_set_line_width(pen.cr, 1.6);
    pen.roundedRect(x + 0.8, top + 0.8, w - 1.6, h - 1.6, 5);
    cairo_stroke(pen.cr);
    pen.roundedRect(x + w + 0.5, cy - 4, 3, 8, 1);
    cairo_fill(pen.cr);
    if (!std::isnan(pct)) {
        const double fw = std::max(2.0, (w - 6) * std::clamp(pct, 0.0, 100.0) / 100.0);
        pen.color(toneOf(level));
        pen.roundedRect(x + 3, top + 3, fw, h - 6, 3);
        cairo_fill(pen.cr);
    }
    std::string s = std::isnan(pct) ? "--" : num(pct, 0) + "%";
    const double size = 14;
    double tw = pen.measure(s, size, true) + (charging ? 11 : 0);
    double tx = x + (w - tw) / 2;
    tx += pen.text(tx, cy + 5, s, size, std::isnan(pct) ? kTextMuted : kText, true);
    if (charging) drawBolt(pen, tx + 1, cy + 6, kWarn);
    cairo_restore(pen.cr);
    return w + 4;
}

// ---- Overview ----

void drawOverviewStrip(const Pen& pen, double x, double y, double w, bool gpu, const WidgetContext& ctx,
                       const History& history, double budgetMs) {
    const Thresholds& th = ctx.thresholds;
    fillTile(pen, x, y, w, kOvStripH);
    const double load = f(ctx, gpu ? kGpuBusyPct : kCpuUsagePct);
    const double temp = f(ctx, gpu ? kTempGpuC : kTempCpuC);
    const bool throttled = i(ctx, gpu ? kThrottleGpu : kThrottleCpu) != 0;
    const Level loadLevel = gpu ? levelOf(load, th.gpuWarnPct, th.gpuCritPct) : levelOf(load, th.cpuWarnPct, th.cpuCritPct);
    pen.text(x + 10, y + 20, gpu ? "GPU" : "CPU", 15, gpu ? kGpu : kAccent, true);
    const double lw = pen.text(x + 10, y + 54, num(load, 0), 34, colorOf(loadLevel), true);
    pen.text(x + 12 + lw, y + 54, "%", 17, kTextMuted);
    pen.text(x + 10, y + 76, std::isnan(temp) ? "-- °C" : num(temp, 0) + " °C", 18,
             colorOf(levelOf(temp, th.tempWarnC, th.tempCritC)));

    // Graph: worst frame time per read, budget dashed
    const double gx = x + 146, gw = w - 146 - 10, gy = y + 26, gh = kOvStripH - 26 - 8;
    pen.color(kBg);
    pen.roundedRect(gx, gy, gw, gh, 6);
    cairo_fill(pen.cr);
    const double maxV = budgetMs > 0 ? budgetMs * 1.5 : 16.7;
    GraphArea area {gx, gy, gw, gh, 0.0, maxV, ctx.now, static_cast<double>(std::max(1, ctx.graphSeconds))};
    if (budgetMs > 0) drawGuide(pen, area, budgetMs, kTextMuted);
    const Color c = gpu ? kGpu : kAccent;
    const double warn = budgetMs > 0 ? budgetMs * th.frameWarnRatio : 1e9;
    const double crit = budgetMs > 0 ? budgetMs * th.frameCritRatio : 1e9;
    drawLevelSeries(pen, area, history, c, warn, crit);

    // Badge above the graph: thermal limit, else CPU capped
    if (throttled) {
        drawTonePill(pen, x + w - 10, y + 4, ctx.language == WidgetLanguage::Ja ? "熱で制限中" : "Thermal limit",
                     kToneRed, true);
    } else if (!gpu) {
        // Capped: the fastest cluster's current max below its hardware max (Playtime or a thermal cap)
        double capped = NAN;
        for (uint16_t k = 0; k < kCpuClusterSlots; ++k) {
            const double mx = f(ctx, kCpuCluster0MaxMhz + k), hw = f(ctx, kCpuCluster0HwMaxMhz + k);
            if (!std::isnan(mx) && !std::isnan(hw) && mx < hw - 1.0) capped = mx;
        }
        if (!std::isnan(capped)) {
            char b[32];
            std::snprintf(b, sizeof(b), "Capped %.1f GHz", capped / 1000.0);
            drawTonePill(pen, x + w - 10, y + 4, b, kToneYellow, true);
        }
    }
    if (throttled) strokeRounded(pen, x + 1.5, y + 1.5, w - 3, kOvStripH - 3, 7, kToneRed, 3.0);
}

void drawOverview(const Pen& pen, double x, double y, const WidgetContext& ctx, const WidgetHistory& h) {
    const Thresholds& th = ctx.thresholds;
    const double innerW = kCardW - 2 * kPad;
    const double leftW = std::round((innerW - kTileGap) * 0.6);
    const double rightW = innerW - kTileGap - leftW;
    const double tx = x + kPad, ty = y + kPad;

    // fps tile
    fillTile(pen, tx, ty, leftW, kOvTopH);
    const double fps = f(ctx, kFrameAppFps);
    const double hz = f(ctx, kFrameDisplayHz);
    const Level fpsLevel = std::isnan(hz) ? Level::Normal
                                          : levelOf(fps, hz * th.fpsWarnRatio, hz * th.fpsCritRatio, true);
    double cx = tx + 10;
    cx += pen.text(cx, ty + 52, std::isnan(fps) ? "--" : num(fps, 0), 52, colorOf(fpsLevel), true);
    cx += 4 + pen.text(cx + 4, ty + 52, "fps", 20, kTextMuted);
    const int64_t throttled = ok(ctx, kFrameAppFps) ? i(ctx, kFrameThrottled) : 0;
    if (throttled > 0) {
        drawTonePill(pen, cx + 8, ty + 36, "1/" + std::to_string(throttled + 1) + " rate", kToneYellow);
    } else if (!std::isnan(hz)) {
        pen.text(cx + 6, ty + 52, "/ " + num(hz, 0) + " Hz", 17, kTextMuted);
    }
    const double rp = f(ctx, kSessionReprojPct);
    const bool haveSession = ok(ctx, kSessionReprojPct);
    double sx = tx + 10;
    const double sb = ty + 80;
    sx += pen.text(sx, sb, "Reprojected ", 15, kTextMuted);
    sx += pen.text(sx, sb, haveSession ? num(rp, rp < 10 ? 1 : 0) + " %" : "--", 17,
                   colorOf(levelOf(rp, th.reprojWarnPct, th.reprojCritPct)), true);
    sx += 16;
    sx += pen.text(sx, sb, "Dropped ", 15, kTextMuted);
    pen.text(sx, sb, ok(ctx, kSessionDropped) ? grouped(static_cast<double>(i(ctx, kSessionDropped))) : "--", 17,
             i(ctx, kSessionDropRecent) != 0 ? kWarn : kText, true);

    // Clock tile: time, then "Game - 42m"
    const double rx = tx + leftW + kTileGap;
    fillTile(pen, rx, ty, rightW, kOvTopH);
    const std::string stamp = text(ctx, kClock);
    std::string hhmm = "--";
    if (stamp.size() >= 16) {
        int hour = std::atoi(stamp.substr(11, 2).c_str());
        const std::string minute = stamp.substr(14, 2);
        if (ctx.clockFormat == 12) {
            const char* ampm = hour >= 12 ? " PM" : " AM";
            hour %= 12;
            if (hour == 0) hour = 12;
            hhmm = std::to_string(hour) + ":" + minute + ampm;
        } else {
            hhmm = stamp.substr(11, 5);
        }
    }
    pen.text(rx + rightW - 10, ty + 42, hhmm, 34, kText, true, true);
    if (ok(ctx, kSceneAppName) || ok(ctx, kSceneAppKey)) {
        const std::string game = ok(ctx, kSceneAppName) ? text(ctx, kSceneAppName) : text(ctx, kSceneAppKey);
        const std::string tail = ok(ctx, kSessionSeconds)
                                     ? " - " + shortDuration(static_cast<double>(i(ctx, kSessionSeconds)))
                                     : "";
        const double tailW = pen.measure(tail, 15);
        const std::string name = ellipsize(pen, game, 15, rightW - 20 - tailW);
        const double nameW = pen.measure(name, 15);
        const double start = rx + rightW - 10 - tailW - nameW;
        pen.text(start, ty + 78, name, 15, kText);
        pen.text(start + nameW, ty + 78, tail, 15, kTextMuted);
    } else {
        pen.text(rx + rightW - 10, ty + 78, "SteamVR", 15, kTextMuted, false, true);
    }

    // CPU and GPU strips
    const double budget = std::isnan(hz) || hz <= 0 ? 0.0 : 1000.0 / hz;
    const double s1 = ty + kOvTopH + kTileGap;
    drawOverviewStrip(pen, tx, s1, innerW, false, ctx, h.c, budget);
    drawOverviewStrip(pen, tx, s1 + kOvStripH + kTileGap, innerW, true, ctx, h.d, budget);
}

// ---- Memory ----

void drawMemory(const Pen& pen, double x, double y, const WidgetContext& ctx) {
    const double used = f(ctx, kRamUsedGib), total = f(ctx, kRamTotalGib);
    double gpu = f(ctx, kGpuMemGib);
    const double pct = (!std::isnan(used) && total > 0) ? 100.0 * used / total : NAN;
    const Level level = levelOf(pct, kRamWarnPct, kRamCritPct);
    const double lx = x + 14, r = x + kCardW - 14;
    const double b1 = y + 50;
    double cx = lx + pen.text(lx, b1, "RAM", 15, kTextMuted) + 14;
    pen.text(cx, b1, std::isnan(pct) ? "--" : num(pct, 0) + " %", 22, colorOf(level), true);
    const std::string totalText = " / " + num(total, 1) + " GB";
    const double totalW = pen.measure(totalText, 15);
    pen.text(r, b1, totalText, 15, kTextMuted, false, true);
    pen.text(r - totalW, b1, num(used, 1) + " GB", 16, colorOf(level), false, true);

    // One bar: the GPU's share first, then everything else (fixed colours; only the numbers warn)
    const double bx = lx, bw = r - lx, by = y + 60, bh = 14;
    pen.color(kBg);
    pen.roundedRect(bx, by, bw, bh, bh / 2);
    cairo_fill(pen.cr);
    if (!std::isnan(used) && total > 0) {
        if (std::isnan(gpu) || gpu > used) gpu = std::isnan(gpu) ? 0.0 : used;
        const double gw = bw * gpu / total;
        const double uw = bw * used / total;
        cairo_save(pen.cr);
        pen.roundedRect(bx, by, std::max(uw, bh), bh, bh / 2);
        cairo_clip(pen.cr);
        pen.color(kGpu);
        cairo_rectangle(pen.cr, bx, by, gw, bh);
        cairo_fill(pen.cr);
        pen.color(kAccent);
        cairo_rectangle(pen.cr, bx + gw + (gw > 0 ? 2 : 0), by, std::max(0.0, uw - gw - (gw > 0 ? 2 : 0)), bh);
        cairo_fill(pen.cr);
        cairo_restore(pen.cr);
    }
    // Legend
    const double b2 = y + 94;
    double lxp = lx;
    const auto key = [&](Color c, const char* label, double gib) {
        pen.color(c);
        pen.roundedRect(lxp, b2 - 10, 10, 10, 3);
        cairo_fill(pen.cr);
        lxp += 16;
        lxp += pen.text(lxp, b2, label, 15, kTextMuted) + 6;
        lxp += pen.text(lxp, b2, std::isnan(gib) ? "--" : num(gib, 1) + " GB", 15, kText, true) + 20;
    };
    key(kGpu, "GPU", gpu);
    key(kAccent, "Apps & system", (std::isnan(used) || std::isnan(gpu)) ? NAN : used - gpu);
}

// ---- Wi-Fi & audio ----

void drawLinkAudio(const Pen& pen, double x, double y, const WidgetContext& ctx) {
    const Thresholds& th = ctx.thresholds;
    const double innerW = kCardW - 2 * kPad;
    const double halfW = (innerW - kTileGap) / 2;
    const double tx = x + kPad, ty = y + kPad, th2 = 84;

    // Link
    fillTile(pen, tx, ty, halfW, th2);
    const bool connected = i(ctx, kNetConnected) != 0;
    if (!connected) {
        iconAntenna(pen, tx + 10, ty + 28, 0, kTextMuted);
        pen.text(tx + 52, ty + 26, "Not connected", 15, kTextMuted);
        pen.text(tx + 10, ty + 66, "--", 18, kTextMuted);
    } else {
        const double dbm = f(ctx, kNetSignalDbm);
        const Level sig = levelOf(dbm, th.wifiWarnDbm, th.wifiCritDbm, true);
        iconAntenna(pen, tx + 10, ty + 28, std::max(1, wifiBars(dbm, th)), colorOf(sig));
        pen.text(tx + 52, ty + 26, i(ctx, kNetHomeWifi) != 0 ? "Wi-Fi" : "Dongle", 15, kTextMuted);
        if (!std::isnan(dbm)) pen.text(tx + halfW - 10, ty + 26, num(dbm, 0) + " dBm", 13, colorOf(sig), false, true);
        double cx = tx + 10;
        const double b = ty + 66;
        for (int k = 0; k < 2; ++k) {
            const RateText r = formatRate(f(ctx, k == 0 ? kNetRxMbps : kNetTxMbps));
            cx += drawArrow(pen, cx, b, k == 1, kTextMuted) + 3;
            cx += pen.text(cx, b, r.value, 21, kText, true);
            cx += pen.text(cx, b, r.unit, 14, kTextMuted) + 14;
        }
    }

    // Audio
    const double ax = tx + halfW + kTileGap;
    fillTile(pen, ax, ty, halfW, th2);
    const double l1 = ty + 30, l2 = ty + 62;
    if (i(ctx, kAudioOutHeadphones) != 0) iconHeadphones(pen, ax + 10, l1 - 6, kText);
    else iconSpeaker(pen, ax + 10, l1 - 5, kText);
    const bool outMuted = i(ctx, kAudioOutMuted) != 0;
    const std::string vol = outMuted ? "Muted" : "";  // no volume, only a mute note
    const double volW = pen.measure(vol, 14);
    pen.text(ax + halfW - 10, l1, vol, 14, outMuted ? kDanger : kTextMuted, false, true);
    pen.text(ax + 32, l1, ellipsize(pen, ok(ctx, kAudioOutName) ? text(ctx, kAudioOutName) : "--", 16,
                                    halfW - 52 - volW), 16, kText);
    const bool micMuted = i(ctx, kAudioInMuted) != 0;
    iconMic(pen, ax + 10, l2 - 5, micMuted);
    const bool haveMic = ok(ctx, kAudioInName);
    double pillW = 0;
    if (haveMic) {
        pillW = drawTonePill(pen, ax + halfW - 10, l2 - 15, micMuted ? "Muted" : "Live", micMuted ? kToneRed : kToneGreen,
                             true, 12.5);
    }
    pen.text(ax + 32, l2, ellipsize(pen, haveMic ? text(ctx, kAudioInName) : "No mic", 16, halfW - 52 - pillW), 16,
             haveMic ? kText : kTextMuted);
}

// ---- Power & batteries ----

/**
 * The weak-charger warning: plugged in, yet the battery has been giving power for the last 15 s and is not nearly
 * full. Near full the charger regulates in and out (5-11 W at 99 %), which would flash the warning on and off.
 */
bool sustainedDrain(const History& battW, double pct, double now) {
    if (!std::isnan(pct) && pct >= 95.0) return false;
    int n = 0;
    double first = now;
    for (const auto& p : battW.points()) {
        if (p.t < now - 15.0) continue;
        if (std::isnan(p.v) || p.v >= -0.3) return false;
        first = std::min(first, p.t);
        ++n;
    }
    return n >= 2 && now - first >= 9.0;
}

void drawPowerBattery(const Pen& pen, double x, double y, const WidgetContext& ctx, const WidgetHistory& h) {
    const Thresholds& th = ctx.thresholds;
    const double lx = x + 14, r = x + kCardW - 14;
    const double draw = f(ctx, kPowerVphW);
    const double bw = pen.text(lx, y + 62, std::isnan(draw) ? "--" : num(draw, 2), 34,
                               colorOf(levelOf(draw, th.powerWarnW, th.powerCritW)), true);
    pen.text(lx + bw + 4, y + 62, "W draw", 17, kTextMuted);

    // USB-C pill and the line under it
    const bool plugged = i(ctx, kBatteryPlugged) != 0;
    const double battW = f(ctx, kBatteryPowerW);  // + into the battery
    const int64_t status = i(ctx, kBatteryStatus);
    const bool full = status == static_cast<int64_t>(BatteryStatus::Full) || text(ctx, kBatteryChargerMode) == "Full";
    const double pillTop = y + 14, pillH = 26;
    if (!plugged) {
        const std::string s = "USB-C not connected";
        const double w = pen.measure(s, 15) + 44;
        pen.color(kControl);
        pen.roundedRect(r - w, pillTop, w, pillH, pillH / 2);
        cairo_fill(pen.cr);
        iconPlug(pen, r - w + 12, pillTop + pillH / 2, kTextMuted, true);
        pen.text(r - w + 34, pillTop + 18, s, 15, kTextMuted);
    } else {
        const double inW = f(ctx, kBatteryInputW);
        const bool draining = !full && sustainedDrain(h.a, f(ctx, kBatteryPct), ctx.now);
        std::string type = text(ctx, kBatteryUsbType);
        const std::string head = "USB-C " + (type.empty() ? std::string("") : type + " ") + "· ";
        const std::string val = std::isnan(inW) ? "--" : num(inW, 1) + " W";
        const double w = pen.measure(head, 15) + pen.measure(val, 15, true) + pen.measure(" in", 15) + 44;
        pen.color(kControl);
        pen.roundedRect(r - w, pillTop, w, pillH, pillH / 2);
        cairo_fill(pen.cr);
        if (draining) strokeRounded(pen, r - w + 0.75, pillTop + 0.75, w - 1.5, pillH - 1.5, pillH / 2, kToneYellow, 1.5);
        iconPlug(pen, r - w + 12, pillTop + pillH / 2, draining ? kWarn : kText, false);
        double px = r - w + 34;
        px += pen.text(px, pillTop + 18, head, 15, draining ? kWarn : kText);
        px += pen.text(px, pillTop + 18, val, 15, draining ? kWarn : kText, true);
        pen.text(px, pillTop + 18, " in", 15, draining ? kWarn : kText);
        // Under it: what the charger offers, and where the power goes
        std::string sub;
        const double offered = f(ctx, kBatteryPdOfferedW);
        if (draining) {
            sub = "less than draw · battery " + num(battW, 1) + " W";
        } else {
            if (!std::isnan(offered)) sub = "offers " + num(offered, 0) + " W · ";
            sub += full ? "battery full" : (std::isnan(battW) ? "" : "battery +" + num(std::max(0.0, battW), 1) + " W");
        }
        pen.text(r, pillTop + pillH + 15, sub, 13, draining ? kWarn : kTextMuted, false, true);
    }

    // Divider, then the batteries
    pen.color(kControl);
    cairo_rectangle(pen.cr, lx, y + 82, r - lx, 1);
    cairo_fill(pen.cr);
    const double cy = y + 104;
    const double pct = f(ctx, kBatteryPct);
    const bool charging = status == static_cast<int64_t>(BatteryStatus::Charging);
    iconHeadset(pen, lx, cy, kTextMuted);
    double bx = lx + 28;
    bx += drawToneBattery(pen, bx, cy, pct, levelOf(pct, th.batteryWarnPct, th.batteryCritPct, true), charging, 60);
    std::string left;
    if (charging && ok(ctx, kBatteryTimeToFullS)) left = shortDuration(f(ctx, kBatteryTimeToFullS)) + " to full";
    else if (!plugged && ok(ctx, kBatteryTimeToEmptyS)) left = shortDuration(f(ctx, kBatteryTimeToEmptyS)) + " remaining";
    if (!left.empty() && left[0] != ' ') pen.text(bx + 6, cy + 5, left, 14, kTextMuted);

    const double lp = f(ctx, kCtrlLeftPct), rp = f(ctx, kCtrlRightPct);
    const double cw = 52;
    double ex = r - (cw + 4) * 2 - 8 - 14 - 16;
    iconController(pen, ex, cy, kTextMuted);
    ex += 20;
    ex += drawToneBattery(pen, ex, cy, lp, levelOf(lp, th.controllerWarnPct, th.controllerCritPct, true), false, cw) + 8;
    drawToneBattery(pen, ex, cy, rp, levelOf(rp, th.controllerWarnPct, th.controllerCritPct, true), false, cw);
}
