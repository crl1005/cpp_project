#include "raylib.h"
#include <vector>
#include <cmath>

static const int SCREEN_W = 960;
static const int SCREEN_H = 540;

// Palette: warm lamplight on a deep ink background
static const Color COL_BG      = {24, 22, 34, 255};
static const Color COL_PLAYER  = {255, 190, 92, 255};
static const Color COL_ORB     = {120, 220, 180, 255};
static const Color COL_HAZARD  = {230, 80, 100, 255};
static const Color COL_TEXT    = {235, 228, 215, 255};
static const Color COL_DIM     = {235, 228, 215, 120};

struct Hazard {
    Vector2 pos;
    Vector2 vel;
    float radius;
};

struct Game {
    Vector2 player;
    float playerRadius = 14.0f;
    float playerSpeed = 280.0f;
    Vector2 orb;
    float orbRadius = 10.0f;
    int score = 0;
    int best = 0;
    bool dead = false;
    std::vector<Hazard> hazards;
};

static Vector2 RandomOrbPos() {
    return { (float)GetRandomValue(60, SCREEN_W - 60),
             (float)GetRandomValue(60, SCREEN_H - 60) };
}

static void SpawnHazard(Game& g) {
    // Enter from a random screen edge, aimed loosely at the middle
    Vector2 p;
    switch (GetRandomValue(0, 3)) {
        case 0:  p = { (float)GetRandomValue(0, SCREEN_W), -20.0f }; break;
        case 1:  p = { (float)GetRandomValue(0, SCREEN_W), SCREEN_H + 20.0f }; break;
        case 2:  p = { -20.0f, (float)GetRandomValue(0, SCREEN_H) }; break;
        default: p = { SCREEN_W + 20.0f, (float)GetRandomValue(0, SCREEN_H) }; break;
    }
    Vector2 target = { SCREEN_W / 2.0f + GetRandomValue(-150, 150),
                       SCREEN_H / 2.0f + GetRandomValue(-100, 100) };
    float dx = target.x - p.x, dy = target.y - p.y;
    float len = std::sqrt(dx * dx + dy * dy);
    float speed = 90.0f + g.score * 5.0f;
    Hazard h;
    h.pos = p;
    h.vel = { dx / len * speed, dy / len * speed };
    h.radius = (float)GetRandomValue(12, 22);
    g.hazards.push_back(h);
}

static void ResetGame(Game& g) {
    g.player = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };
    g.orb = RandomOrbPos();
    g.score = 0;
    g.dead = false;
    g.hazards.clear();
    SpawnHazard(g);
}

static void Update(Game& g, float dt) {
    if (g.dead) {
        if (IsKeyPressed(KEY_R)) ResetGame(g);
        return;
    }

    // Movement
    Vector2 dir = { 0, 0 };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dir.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dir.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir.x += 1;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0) {
        g.player.x += dir.x / len * g.playerSpeed * dt;
        g.player.y += dir.y / len * g.playerSpeed * dt;
    }
    if (g.player.x < g.playerRadius) g.player.x = g.playerRadius;
    if (g.player.x > SCREEN_W - g.playerRadius) g.player.x = SCREEN_W - g.playerRadius;
    if (g.player.y < g.playerRadius) g.player.y = g.playerRadius;
    if (g.player.y > SCREEN_H - g.playerRadius) g.player.y = SCREEN_H - g.playerRadius;

    // Collect orb
    if (CheckCollisionCircles(g.player, g.playerRadius, g.orb, g.orbRadius)) {
        g.score++;
        if (g.score > g.best) g.best = g.score;
        g.orb = RandomOrbPos();
        SpawnHazard(g);
    }

    // Hazards drift and bounce once they've entered the arena
    for (Hazard& h : g.hazards) {
        h.pos.x += h.vel.x * dt;
        h.pos.y += h.vel.y * dt;
        bool inside = h.pos.x > 0 && h.pos.x < SCREEN_W && h.pos.y > 0 && h.pos.y < SCREEN_H;
        if (inside) {
            if (h.pos.x < h.radius || h.pos.x > SCREEN_W - h.radius) h.vel.x = -h.vel.x;
            if (h.pos.y < h.radius || h.pos.y > SCREEN_H - h.radius) h.vel.y = -h.vel.y;
        }
        if (CheckCollisionCircles(g.player, g.playerRadius - 3, h.pos, h.radius - 2)) {
            g.dead = true;
        }
    }
}

static void Draw(const Game& g) {
    ClearBackground(COL_BG);

    float t = (float)GetTime();

    // Orb with a breathing halo
    float pulse = 4.0f + std::sin(t * 4.0f) * 3.0f;
    DrawCircleLinesV(g.orb, g.orbRadius + pulse, Fade(COL_ORB, 0.5f));
    DrawCircleV(g.orb, g.orbRadius, COL_ORB);

    for (const Hazard& h : g.hazards) {
        DrawCircleV(h.pos, h.radius, COL_HAZARD);
        DrawCircleLinesV(h.pos, h.radius + 3, Fade(COL_HAZARD, 0.4f));
    }

    DrawCircleV(g.player, g.playerRadius, COL_PLAYER);

    DrawText(TextFormat("%d", g.score), 24, 18, 40, COL_TEXT);
    DrawText(TextFormat("best %d", g.best), 24, 62, 18, COL_DIM);

    if (g.dead) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(COL_BG, 0.75f));
        const char* msg = "you got caught";
        int w = MeasureText(msg, 44);
        DrawText(msg, (SCREEN_W - w) / 2, SCREEN_H / 2 - 40, 44, COL_TEXT);
        const char* sub = "press R to try again";
        int sw = MeasureText(sub, 20);
        DrawText(sub, (SCREEN_W - sw) / 2, SCREEN_H / 2 + 20, 20, COL_DIM);
    }
}

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "mygame");
    SetTargetFPS(60);

    Game game;
    ResetGame(game);

    while (!WindowShouldClose()) {
        Update(game, GetFrameTime());
        BeginDrawing();
        Draw(game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
