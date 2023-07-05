#include "projects/wolf/vulkan_compute/wolf_renderer_vulkan_compute.hpp"

#include "tools/math/math.hpp"

#include <vulkan/vulkan.hpp>

#include <glm/vec3.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace {
class PrepareWallsThreadCallable {
public:
  PrepareWallsThreadCallable(const std::size_t                                                     id,
                             const std::size_t                                                     ray_first,
                             const std::size_t                                                     ray_one_past_last,
                             wolf::WolfRendererMultithread::CommonPrepareWallsThreadCallableState &common_state)
      : id_{id}, ray_first_{ray_first}, ray_one_past_last_{ray_one_past_last}, common_state_{common_state} {}

  void operator()() {
    constexpr auto ray_length{100.0f};

    std::unique_lock lk(common_state_.m);

    const auto &dir = common_state_.player_state.dir();
    const auto &pos = common_state_.player_state.pos();
    do {
      common_state_.cv.wait(lk, [&]() { return common_state_.do_work.at(id_); });
      common_state_.do_work.at(id_) = false;
      if (common_state_.join) { break; }
      lk.unlock();

      for (auto r_it = ray_first_; r_it < ray_one_past_last_; r_it++) {
        const auto ray_cos = common_state_.ray_rots.at(r_it).cos;
        const auto ray_sin = common_state_.ray_rots.at(r_it).sin;
        const auto ray_x   = (ray_cos * dir.x - ray_sin * dir.y) * ray_length + pos.x;
        const auto ray_y   = (ray_sin * dir.x + ray_cos * dir.y) * ray_length + pos.y;

        auto v_index  = std::numeric_limits<std::size_t>::max();
        auto min_dist = ray_length * 2.0f;
        auto min_x    = 0.0f;
        auto min_y    = 0.0f;
        for (auto v_it = std::size_t{0u}; v_it < common_state_.vector_map.vectors().size(); v_it++) {
          const auto &v = common_state_.vector_map.vectors().at(v_it);

          if (!tools::math::do_intersect(pos.x, pos.y, ray_x, ray_y, v.first.x, v.first.y, v.second.x, v.second.y)) {
            continue;
          }
          const auto [crossed, cross_x, cross_y] =
              tools::math::intersection_point(pos.x, pos.y, ray_x, ray_y, v.first.x, v.first.y, v.second.x, v.second.y);
          if (!crossed) { continue; }

          const auto curr_dist =
              std::sqrtf((cross_x - pos.x) * (cross_x - pos.x) + (cross_y - pos.y) * (cross_y - pos.y));
          if (curr_dist < min_dist) {
            min_dist = curr_dist;
            min_x    = cross_x;
            min_y    = cross_y;
            v_index  = v_it;
          }
        }

        auto cam_dist = std::numeric_limits<float>::max();
        if (v_index != std::numeric_limits<std::size_t>::max()) {
          cam_dist = (common_state_.cam_2.x - common_state_.cam_1.x) * (common_state_.cam_1.y - min_y) -
                     (common_state_.cam_1.x - min_x) * (common_state_.cam_2.y - common_state_.cam_1.y);
          cam_dist /= std::sqrtf(
              (common_state_.cam_2.x - common_state_.cam_1.x) * (common_state_.cam_2.x - common_state_.cam_1.x) +
              (common_state_.cam_2.y - common_state_.cam_1.y) * (common_state_.cam_2.y - common_state_.cam_1.y));
          if (cam_dist < 0.0f) { cam_dist = 0.0f; }
        }

        common_state_.walls.at(r_it).rect.h      = 1.0f / cam_dist * common_state_.height;
        common_state_.walls.at(r_it).rect.y      = (common_state_.height - common_state_.walls.at(r_it).rect.h) / 2.0f;
        common_state_.walls.at(r_it).color_index = v_index == std::numeric_limits<std::size_t>::max() ? 0u : v_index;
        common_state_.walls.at(r_it).shadow_factor =
            1.0f - std::min(0.75f, static_cast<float>(common_state_.walls.at(r_it).rect.h) / common_state_.height);
      }

      lk.lock();
      common_state_.done++;
      common_state_.cv_main.notify_one();
    } while (true);
  }

private:
  const std::size_t                                                     id_{};
  const std::size_t                                                     ray_first_{};
  const std::size_t                                                     ray_one_past_last_{};
  wolf::WolfRendererMultithread::CommonPrepareWallsThreadCallableState &common_state_;
};
} // namespace

namespace wolf {
WolfRendererMultithread::WolfRendererMultithread(tools::sdl::SDLSystem                   &sdl_sys,
                                                 const std::shared_ptr<const VectorMap>   vector_map,
                                                 const std::shared_ptr<const PlayerState> player_state,
                                                 const std::uint32_t                      rays,
                                                 const std::uint32_t                      threads)
    : WolfRenderer(sdl_sys, vector_map, player_state, rays)
    , common_state_{height_, *player_state_, *vector_map_, ray_rots_, walls_} {
  common_state_.do_work = std::vector<bool>(threads, false);

  const auto rays_per_thread = static_cast<float>(ray_rots_.size()) / threads;
  threads_.reserve(threads);
  for (auto t_it = std::size_t{0u}; t_it < threads; t_it++) {
    const auto ray_first = static_cast<std::size_t>(std::floor(t_it * rays_per_thread));

    const auto last_thread = t_it == threads - 1;
    const auto ray_one_past_last =
        last_thread ? ray_rots_.size() : static_cast<std::size_t>(std::floor((t_it + 1) * rays_per_thread));

    threads_.emplace_back(PrepareWallsThreadCallable{t_it, ray_first, ray_one_past_last, common_state_});
  }

  const auto application_info = vk::ApplicationInfo()
                                    .setPApplicationName("list_extensions")
                                    .setApplicationVersion(0)
                                    .setEngineVersion(0)
                                    .setPEngineName("rules_vulkan");

  const auto instance_create_info = vk::InstanceCreateInfo().setPApplicationInfo(&application_info);

  vk::Instance instance;
  vk::Result   result = vk::createInstance(&instance_create_info, nullptr, &instance);
  if (result != vk::Result::eSuccess) { printf("error creating vulkan instance\n"); }

  auto instance_extension_properties = vk::enumerateInstanceExtensionProperties();
  for (const auto &instance_extension_property : instance_extension_properties) {
    printf("%s\n", instance_extension_property.extensionName);
  }
}

WolfRendererMultithread::~WolfRendererMultithread() {
  common_state_.join = true;
  std::fill(common_state_.do_work.begin(), common_state_.do_work.end(), true);
  common_state_.cv.notify_all();
  for (auto t_it = std::size_t{0u}; t_it < threads_.size(); t_it++) { threads_.at(t_it).join(); }
}

void WolfRendererMultithread::prepare_walls() {
  const auto cam_cos1 = std::cosf(-std::numbers::pi / 2.0f);
  const auto cam_sin1 = std::sinf(-std::numbers::pi / 2.0f);
  const auto cam_cos2 = std::cosf(std::numbers::pi / 2.0f);
  const auto cam_sin2 = std::sinf(std::numbers::pi / 2.0f);

  const auto &dir = common_state_.player_state.dir();
  const auto &pos = common_state_.player_state.pos();

  common_state_.cam_1 =
      glm::vec2{(cam_cos1 * dir.x - cam_sin1 * dir.y) + pos.x, (cam_sin1 * dir.x + cam_cos1 * dir.y) + pos.y};
  common_state_.cam_2 =
      glm::vec2{(cam_cos2 * dir.x - cam_sin2 * dir.y) + pos.x, (cam_sin2 * dir.x + cam_cos2 * dir.y) + pos.y};

  common_state_.done = 0u;
  std::fill(common_state_.do_work.begin(), common_state_.do_work.end(), true);
  common_state_.cv.notify_all();

  std::unique_lock lk(common_state_.m);
  common_state_.cv_main.wait(lk, [&]() { return common_state_.done == threads_.size(); });
}
} // namespace wolf
