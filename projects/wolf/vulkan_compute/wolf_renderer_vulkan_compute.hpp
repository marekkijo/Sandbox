#pragma once

#include "projects/wolf/player_state.hpp"
#include "projects/wolf/vector_map.hpp"
#include "projects/wolf/wolf_renderer.hpp"

#include "tools/sdl/sdl_system.hpp"

#include <glm/vec2.hpp>

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace wolf {
class WolfRendererMultithread final : public WolfRenderer {
public:
  WolfRendererMultithread(const WolfRendererMultithread &)                = default;
  WolfRendererMultithread &operator=(const WolfRendererMultithread &)     = default;
  WolfRendererMultithread(WolfRendererMultithread &&) noexcept            = default;
  WolfRendererMultithread &operator=(WolfRendererMultithread &&) noexcept = default;

  WolfRendererMultithread(tools::sdl::SDLSystem                   &sdl_sys,
                          const std::shared_ptr<const VectorMap>   vector_map,
                          const std::shared_ptr<const PlayerState> player_state,
                          const std::uint32_t                      rays,
                          const std::uint32_t                      threads);
  ~WolfRendererMultithread() final;

public:
  struct CommonPrepareWallsThreadCallableState {
    const int                  height;
    const PlayerState         &player_state;
    const VectorMap           &vector_map;
    const std::vector<RayRot> &ray_rots;

    std::vector<Wall> &walls;

    glm::vec2 cam_1{};
    glm::vec2 cam_2{};

    std::mutex              m{};
    std::condition_variable cv{};
    std::condition_variable cv_main{};
    std::vector<bool>       do_work{};
    std::uint32_t           done{0u};
    bool                    join{false};
  };

private:
  CommonPrepareWallsThreadCallableState common_state_;
  std::vector<std::thread>              threads_{};

  void prepare_walls() final;
};
} // namespace wolf
