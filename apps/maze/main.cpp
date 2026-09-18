//this is maze, it is hosted on the core sdl3 webgpu app host, todo 8 of the core app host plan
//the window, gpu, and imgui boot along with the frame loop all live in core/sources/app, so this
//file just maps the game onto the host callbacks, World::Start runs once in on_init, then Update(dt)
//runs per frame in on_iterate and OnDraw and OnGui run inside the host's open imgui frame in on_draw,
//which is the same order the old hand rolled loop used, update then ondraw then ongui
#include "app/sdl_app.hpp"
#include "imgui/imgui_layer.hpp"
#include "World.h"
#include "MazeGame.h"

#include <SDL3/SDL_log.h>

#include <cstdlib>
#include <cstring>

namespace {

  struct MazeApp : app::AppCallbacks {
    app::ImGuiLayer imgui_layer;
    World mazeWorld{21};
    MazeGame mazeGame;  //this is the playable layer on top of mazeWorld, check MazeGame.h
    int smoke_frames = 0;  //--smoke-frames N: deterministic exit-0 headless smoke

    SDL_AppResult on_init(app::App& app, int argc, char** argv) override {
      app.settings.title = "Maze";
      const float clear[4] = {0.05f, 0.05f, 0.05f, 1.00f};
      for (int i = 0; i < 4; ++i) app.settings.clear_color[i] = clear[i];
      app::AppSettings::parse(argc, argv, app.settings);
      for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--smoke-frames") == 0 && i + 1 < argc) smoke_frames = std::atoi(argv[++i]);

      //HeadlessNone does not have a device so ImGuiLayer::init would fail on that path, so it just
      //runs the pure logic loop without a gui layer instead
      if (app.settings.render_mode != app::AppSettings::RenderMode::HeadlessNone) app.attach_gui(imgui_layer);

      SDL_Log("Creating Maze World");
      mazeWorld.Start();
      SDL_Log("Maze World Started");
      return SDL_APP_CONTINUE;
    }

    //this is logic only, HeadlessNone never calls on_draw so Update has to live here
    SDL_AppResult on_iterate(app::App& app, float dt) override {
      //it always lets the generator run and finish normally now, play mode being on does not block
      //it anymore, MazeGame::Update() is a safe no op unless play mode is active and a game is actually
      //going, so it is fine to call both every frame even while a maze is still generating
      mazeWorld.Update(dt);
      mazeGame.Update(dt, mazeWorld);

      if (smoke_frames > 0 && --smoke_frames == 0)
      {
        app.request_exit();
        return SDL_APP_SUCCESS;
      }
      return SDL_APP_CONTINUE;
    }

    //this is the gui plus the background draw list, it happens inside the imgui frame the host opened
    void on_draw(app::App& app, WGPURenderPassEncoder pass) override {
      (void)app;
      (void)pass;
      mazeWorld.OnDraw();  //it is unchanged, it renders the walls plus whatever is in the color
                            //buffer, which is also what MazeGame paints into with SetNodeColor
      mazeWorld.OnGui();   //it is unchanged, this is the existing generator dropdown and start pause step reset
      mazeGame.OnGui(mazeWorld);  //this is the new play maze panel, it is its own window
    }
  };

}  //namespace

MOBAGEN_MAIN(MazeApp)
