#include "projects/wolf/vulkan_compute/wolf_renderer_vulkan_compute.hpp"

#include "projects/wolf/map_renderer.hpp"
#include "projects/wolf/player_state.hpp"
#include "projects/wolf/raw_map.hpp"
#include "projects/wolf/raw_map_from_ascii.hpp"
#include "projects/wolf/raw_map_from_wolf.hpp"
#include "projects/wolf/vector_map.hpp"

#include "tools/sdl/sdl_animation.hpp"
#include "tools/sdl/sdl_system.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <boost/program_options.hpp>

#include <cstdint>
#include <iostream>
#include <memory>

struct ProgramSetup {
  bool exit{};

  std::string   asciimap{};
  std::string   maphead{};
  std::string   gamemaps{};
  int           width{};
  int           height{};
  std::uint32_t rays{};
  std::uint32_t threads{};
};

ProgramSetup process_args(const int argc, const char *const argv[]) {
  boost::program_options::options_description desc("Wolf options");
  desc.add_options()("help", "This help message");
  desc.add_options()("asciimap",
                     boost::program_options::value<std::string>()->default_value("projects/wolf/data/map1.map"),
                     "Ascii map filename");
  desc.add_options()("maphead",
                     boost::program_options::value<std::string>()->default_value("projects/wolf/data/MAPHEAD.WL6"),
                     "Wolf MAPHEAD filename");
  desc.add_options()("gamemaps",
                     boost::program_options::value<std::string>()->default_value("projects/wolf/data/GAMEMAPS.WL6"),
                     "Wolf GAMEMAPS filename");
  desc.add_options()("width", boost::program_options::value<int>()->default_value(1520), "Width of the window");
  desc.add_options()("height", boost::program_options::value<int>()->default_value(760), "Height of the window");
  desc.add_options()("rays", boost::program_options::value<std::uint32_t>()->default_value(152u), "Number of rays");
  desc.add_options()("threads",
                     boost::program_options::value<std::uint32_t>()->default_value(32u),
                     "Number of threads");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
    desc.print(std::cout);
    return {true};
  }

  return {false,
          vm["asciimap"].as<std::string>(),
          vm["maphead"].as<std::string>(),
          vm["gamemaps"].as<std::string>(),
          vm["width"].as<int>(),
          vm["height"].as<int>(),
          vm["rays"].as<std::uint32_t>(),
          vm["threads"].as<std::uint32_t>()};
}

int main(int argc, char *argv[]) {
  const auto program_setup = process_args(argc, argv);
  if (program_setup.exit) { return 1; }

  const auto wnd_title = "Wolf: Vulkan Compute";

  auto sdl_sys = tools::sdl::SDLSystem{SDL_INIT_EVERYTHING,
                                       wnd_title,
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       program_setup.width,
                                       program_setup.height,
                                       SDL_WINDOW_RESIZABLE,
                                       -1,
                                       SDL_RENDERER_SOFTWARE};

  auto       raw_map_from_wolf = wolf::RawMapFromWolf{program_setup.maphead, program_setup.gamemaps};
  const auto raw_map           = std::shared_ptr(std::move(raw_map_from_wolf.create_map(0u)));

  // auto       raw_map_from_ascii = wolf::RawMapFromAscii{program_setup.asciimap};
  // const auto raw_map            = std::shared_ptr(std::move(raw_map_from_ascii.create_map()));

  const auto vector_map = std::make_shared<wolf::VectorMap>(std::const_pointer_cast<const wolf::RawMap>(raw_map));
  const auto player_state =
      std::make_shared<wolf::PlayerState>(std::const_pointer_cast<const wolf::RawMap>(raw_map), 90.0f, 3.0f, 2.0f);

  auto map_renderer = wolf::MapRenderer{sdl_sys,
                                        std::const_pointer_cast<const wolf::VectorMap>(vector_map),
                                        std::const_pointer_cast<const wolf::PlayerState>(player_state),
                                        false};

  auto wolf_renderer = wolf::WolfRendererMultithread{sdl_sys,
                                                     std::const_pointer_cast<const wolf::VectorMap>(vector_map),
                                                     std::const_pointer_cast<const wolf::PlayerState>(player_state),
                                                     program_setup.rays,
                                                     program_setup.threads};

  auto last_timestamp_ms = SDL_GetTicks();
  auto animation         = tools::sdl::SDLAnimation(30u);
  auto quit              = false;

  while (!quit) {
    SDL_Event event;
    while (!quit && SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: quit = true; break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          wolf_renderer.Renderer::resize();
          map_renderer.Renderer::resize();
          break;
        default: break;
        }
        break;
      case SDL_USEREVENT:
        SDL_PumpEvents();
        {
          const auto time_elapsed_ms = event.user.timestamp - last_timestamp_ms;
          player_state->animate(time_elapsed_ms);
          last_timestamp_ms = event.user.timestamp;
        }
        SDL_SetRenderDrawColor(sdl_sys.r(), 0, 0, 0, 255);
        SDL_RenderClear(sdl_sys.r());
        wolf_renderer.redraw();
        map_renderer.redraw();
        SDL_RenderPresent(sdl_sys.r());
        break;
      default: break;
      }
    }
  }

  return 0;
}
