#include <SDL2/SDL.h>
#include <stdbool.h>

// Must be closed
static const int map[][13] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

struct point
{
    double x, y;
};

static double mag(const struct point point)
{
    const double x = pow(point.x, 2.0);
    const double y = pow(point.y, 2.0);
    return sqrt(x + y);
}

static struct point sub(const struct point i, const struct point j)
{
    struct point temp = i;
    temp.x -= j.x;
    temp.y -= j.y;
    return temp;
}

static struct point sn(const struct point player, const double m, const double b)
{
    const double y = ceil(player.y - 1);
    const double x = (y - b) / m;
    return (struct point){ x, y };
}

static struct point se(const struct point player, const double m, const double b)
{
    const double x = floor(player.x + 1);
    const double y = m * x + b;
    return (struct point){ x, y };
}

static struct point ss(const struct point player, const double m, const double b)
{
    const double y = floor(player.y + 1);
    const double x = (y - b) / m;
    return (struct point){ x, y };
}

static struct point sw(const struct point player, const double m, const double b)
{
    const double x = ceil(player.x - 1);
    const double y = m * x + b;
    return (struct point){ x, y };
}

static bool hor(const struct point point)
{
    const int x = point.x;
    const int y = point.y;
    return (point.y == floor(point.y)) && (map[y][x] || map[y - 1][x]);
}

static bool ver(const struct point point)
{
    const int x = point.x;
    const int y = point.y;
    return (point.x == floor(point.x)) && (map[y][x] || map[y][x - 1]);
}

static bool col(const struct point player)
{
    const int x = player.x;
    const int y = player.y;
    return map[y][x];
}

static double where(const struct point point)
{
    double null;
    if(hor(point)) return modf(point.x, &null);
    if(ver(point)) return modf(point.y, &null);
    return -1.0;
}

static struct point step(const struct point player, const double m, const int quadrant)
{
    const double b = player.y - m * player.x;
    const struct point n = sn(player, m, b); // Step north
    const struct point e = se(player, m, b); // Step east
    const struct point s = ss(player, m, b); // Step south
    const struct point w = sw(player, m, b); // Step west
    // Step to the next line
    struct point point;
    switch(quadrant)
    {
        case 0: point = mag(sub(e, player)) < mag(sub(s, player)) ? e : s; break;
        case 1: point = mag(sub(w, player)) < mag(sub(s, player)) ? w : s; break;
        case 2: point = mag(sub(w, player)) < mag(sub(n, player)) ? w : n; break;
        case 3: point = mag(sub(e, player)) < mag(sub(n, player)) ? e : n; break;
    }
    // Move onto the next line if no wall is found
    return hor(point) || ver(point) ? point : step(point, m, quadrant);
}

static int quadrant(const double radians)
{
    const double x = cos(radians);
    const double y = sin(radians);
    if(x >= 0.0 && y >= 0.0) return 0;
    if(x <= 0.0 && y >= 0.0) return 1;
    if(x <= 0.0 && y <= 0.0) return 2;
    if(x >= 0.0 && y <= 0.0) return 3;
    return -1;
}

int main(void)
{
    const int xres = 800;
    const int yres = 600;
    // SDL init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* const window = SDL_CreateWindow("water", 120, 80, xres, yres, SDL_WINDOW_SHOWN);
    SDL_Renderer* const renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Surface* const surface = SDL_LoadBMP("hello.bmp");
    // Player init
    struct point player = { 2.5, 2.5 };
    double theta = 0.0;
    const double d0 = 0.025;
    const double dy = 0.025;
    const double dx = 0.025;
    // Game loop
    const uint8_t* const key = SDL_GetKeyboardState(NULL);
    for(;;)
    {
        const int t0 = SDL_GetTicks();
        // Keyboard update
        SDL_PumpEvents();
        // Keyboard exit
        if(key[SDL_SCANCODE_LCTRL] && key[SDL_SCANCODE_D]) break;
        // Keyboard rotation
        if(key[SDL_SCANCODE_H]) theta -= d0;
        if(key[SDL_SCANCODE_L]) theta += d0;
        // Keyboard movement
        struct point temp = player;
        if(key[SDL_SCANCODE_W]) temp.x += dx * cos(theta), temp.y += dy * sin(theta);
        if(key[SDL_SCANCODE_S]) temp.x -= dx * cos(theta), temp.y -= dy * sin(theta);
        if(key[SDL_SCANCODE_A]) temp.y -= dx * cos(theta), temp.x += dy * sin(theta);
        if(key[SDL_SCANCODE_D]) temp.y += dx * cos(theta), temp.x -= dy * sin(theta);
        player = col(temp) ? player : temp;
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        // Buffer columns
        for(int col = 0; col < xres; col++)
        {
            const double pan = 2.0 * (double)col / xres - 1.0;
            const double focal = 2.5;
            const double sigma = atan2(pan, focal);
            const double radians = sigma + theta;
            const double m = tan(radians);
            const struct point wall = step(player, m, quadrant(radians));
            const struct point ray = sub(wall, player);
            // Fish eye correction
            const double magnitude = mag(ray);
            const double normal = magnitude * cos(sigma);
            // Wall height
            const double height = yres / normal;
            const double top = (yres / 2.0) - (height / 2.0);
            const double bot = (yres / 2.0) + (height / 2.0);
            // Texture mapping
            const double offset = where(wall);
            const double percentage = 100.0 * offset;
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0x00);
            SDL_RenderDrawLine(renderer, col, top, col, bot);
        }
        // Render wall
        SDL_RenderPresent(renderer);
        const int t1 = SDL_GetTicks();
        const int dt = t1 - t0;
        // 100 Fps
        const int ms = 10 - dt < 0 ? 0 : dt;
        SDL_Delay(ms);
    }
    // Cleanup
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
