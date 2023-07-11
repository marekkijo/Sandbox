#include "tools/sdl/sdl_animation.hpp"
#include "tools/sdl/sdl_system.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <boost/program_options.hpp>

#include <cstdint>
#include <iostream>
#include <memory>

struct ProgramSetup {
  bool exit{};

  int width{};
  int height{};
};

ProgramSetup process_args(const int argc, const char *const argv[]) {
  boost::program_options::options_description desc("Wolf options");
  desc.add_options()("help", "This help message");
  desc.add_options()("width", boost::program_options::value<int>()->default_value(512), "Width of the frame buffer");
  desc.add_options()("height", boost::program_options::value<int>()->default_value(384), "Height of the frame buffer");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
    desc.print(std::cout);
    return {true};
  }

  return {false, vm["width"].as<int>(), vm["height"].as<int>()};
}

int main(int argc, char *argv[]) {
  const auto program_setup = process_args(argc, argv);
  if (program_setup.exit) { return 1; }

  const auto wnd_title = "OpenGL Streamer";

  auto sdl_sys    = tools::sdl::SDLSystem{SDL_INIT_EVERYTHING,
                                       wnd_title,
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       program_setup.width,
                                       program_setup.height,
                                       SDL_WINDOW_OPENGL,
                                       -1,
                                       SDL_RENDERER_SOFTWARE};
  auto gl_context = SDL_GL_CreateContext(sdl_sys.wnd());

  auto last_timestamp_ms = SDL_GetTicks();
  auto animation         = tools::sdl::SDLAnimation(30u);
  auto quit              = false;

  glReadBuffer(GL_BACK);

  auto buffer_data = std::vector<std::uint8_t>(program_setup.width * program_setup.height * 3);

  while (!quit) {
    SDL_Event event;
    while (!quit && SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: quit = true; break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          int width, height;
          SDL_GetWindowSize(sdl_sys.wnd(), &width, &height);
          glViewport(0, 0, width, height);
        } break;
        default: break;
        }
        break;
      case SDL_USEREVENT:
        SDL_PumpEvents();
        {
          const auto time_elapsed_ms = event.user.timestamp - last_timestamp_ms;

          // renderer->animate(time_elapsed_ms);

          last_timestamp_ms = event.user.timestamp;
        }
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glReadPixels(0, 0, program_setup.width, program_setup.height, GL_RGB, GL_UNSIGNED_BYTE, buffer_data.data());
        printf("RGB(%i, %i, %i)\n", buffer_data[0], buffer_data[1], buffer_data[2]);
        // SDL_GL_SwapWindow(sdl_sys.wnd());

        break;
      default: break;
      }
    }
  }

  return 0;
}
