#include "raw_map_from_wolf.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace wolf {
RawMapFromWolf::RawMapFromWolf(const std::string &maphead_filename, const std::string &gamemaps_filename)
    : gamemaps_file_{gamemaps_filename, std::ios::binary | std::ios::in} {
  auto maphead_file = std::ifstream{maphead_filename};

  if (!maphead_file.is_open() || !gamemaps_file_.is_open()) {
    throw std::runtime_error{"cannot open given map file(s)"};
  }

#pragma pack(push, 1)

  struct {
    std::uint16_t RLEWtag;
    std::int32_t  headeroffsets[100];
    /* skipped:
    std::uint8_t  tileinfo[];
    */
  } map_file_type;

#pragma pack(pop)

  maphead_file.read(reinterpret_cast<char *>(&map_file_type), sizeof(map_file_type));

  rlew_tag_ = map_file_type.RLEWtag;
  printf("%i\n", rlew_tag_);
  for (const auto &v : map_file_type.headeroffsets) {
    if (v > 0) { header_offsets_.push_back(v); }
  }
}

std::size_t RawMapFromWolf::maps_size() { return header_offsets_.size(); }

std::unique_ptr<RawMap> RawMapFromWolf::create_map(const std::size_t map_index) {
#pragma pack(push, 1)

  struct {
    std::int32_t  planestart[3];
    std::uint16_t planelength[3];
    std::uint16_t width, height;
    char          name[16];
  } map_type;

#pragma pack(pop)

  gamemaps_file_.seekg(header_offsets_.at(map_index));
  gamemaps_file_.read(reinterpret_cast<char *>(&map_type), sizeof(map_type));

  std::size_t width{map_type.width};
  std::size_t height{map_type.height};

  auto compressed_planes = std::array<std::vector<std::uint16_t>, 3>{};
  for (std::size_t p_it{0u}; p_it < compressed_planes.size(); p_it++) {
    compressed_planes[p_it].resize(map_type.planelength[p_it]);
    gamemaps_file_.seekg(map_type.planestart[p_it]);
    gamemaps_file_.read(reinterpret_cast<char *>(compressed_planes[p_it].data()),
                        sizeof(compressed_planes[p_it][0]) * compressed_planes[p_it].size());
  }

  auto blocks = std::vector<RawMap::BlockType>(width * height);
  decarmacize_plane(compressed_planes[0], blocks, 0);
  decarmacize_plane(compressed_planes[1], blocks, 1);
  decarmacize_plane(compressed_planes[2], blocks, 2);

  printf("name: %s\n", map_type.name);

  for (std::size_t b_it{0u}; b_it < blocks.size(); b_it++) {
    if ((blocks[b_it][0] >= 1 && blocks[b_it][0] <= 12) || (blocks[b_it][0] >= 14 && blocks[b_it][0] <= 20)) {
      blocks[b_it][0] = 1;
    } else {
      blocks[b_it][0] = 0;
    }

    if (blocks[b_it][1] == 19) {
      blocks[b_it][0] = 'n';
    } else if (blocks[b_it][1] == 21) {
      blocks[b_it][0] = 's';
    } else if (blocks[b_it][1] == 22) {
      blocks[b_it][0] = 'w';
    } else if (blocks[b_it][1] == 20) {
      blocks[b_it][0] = 'e';
    }
  }

  return std::make_unique<RawMap>(width, height, std::move(blocks));
}

void RawMapFromWolf::decarmacize_plane(const std::vector<std::uint16_t> &plane_data,
                                       std::vector<RawMap::BlockType>   &blocks,
                                       [[maybe_unused]] std::size_t      plane) const {
  constexpr auto NEARTAG = std::uint16_t{0xa7};
  constexpr auto FARTAG  = std::uint16_t{0xa8};

  auto                extended_plane = std::vector<std::uint16_t>(plane_data.at(0u) / 2u);
  const std::uint8_t *in_ptr         = reinterpret_cast<const std::uint8_t *>(&plane_data.at(1u));

  for (std::size_t e_it{0u}; e_it < extended_plane.size(); e_it++) {
    std::uint16_t ch;
    std::memcpy(&ch, in_ptr, 2);
    in_ptr += 2;

    if ((ch >> 8u) == NEARTAG) {
      auto count = ch & 0xff;

      if (count == 0u) {
        extended_plane[e_it] = ch | *in_ptr++;
      } else {
        std::uint16_t offset = *in_ptr++;
        for (std::size_t it{0u}; it < count; it++) { extended_plane[e_it + it] = extended_plane[(e_it - offset) + it]; }
        e_it += count - 1u;
      }
    } else if ((ch >> 8u) == FARTAG) {
      auto count = ch & 0xff;

      if (count == 0u) {
        extended_plane[e_it] = ch | *in_ptr++;
      } else {
        std::uint16_t offset;
        memcpy(&offset, in_ptr, 2);
        in_ptr += 2;
        for (std::size_t it{0u}; it < count; it++) { extended_plane[e_it + it] = extended_plane[offset + it]; }
        e_it += count - 1u;
      }
    } else {
      extended_plane[e_it] = ch;
    }
  }

  expand_plane(extended_plane, blocks, plane);
}

void RawMapFromWolf::expand_plane(const std::vector<std::uint16_t> &plane_data,
                                  std::vector<RawMap::BlockType>   &blocks,
                                  [[maybe_unused]] std::size_t      plane) const {
  for (std::size_t b_it{0u}, p_it{1u}; b_it < blocks.size(); b_it++) {
    blocks[b_it][plane] = plane_data.at(p_it++);

    if (blocks[b_it][plane] != rlew_tag_) { continue; }

    const auto count = plane_data.at(p_it++);
    const auto value = plane_data.at(p_it++);
    if (count != 0) {
      for (auto it = 0; it < count; it++) { blocks[b_it + it][plane] = value; }
    }
    b_it += count - 1u;
  }
}
} // namespace wolf