#ifndef _SDL_TTF_MULTIFONT_H_
#define _SDL_TTF_MULTIFONT_H_

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

/* Rendering with font fallback for SDL_ttf. */

class Fonts {
 public:
  explicit Fonts(std::vector<TTF_Font *> fonts);
  Fonts(Fonts &&) = default;
  Fonts &operator=(Fonts &&) noexcept = default;

  // Not thread-safe.
  TTF_Font *GetFontForCodePoint(std::uint16_t code_point) const;

  bool IsSingle() const {
    return fonts_.size() == 1;
  }

  TTF_Font *GetFirstFont() const {
      return fonts_[0];
  }

  std::vector<TTF_Font *> &fonts() {
    return fonts_;
  }

 private:
  std::vector<TTF_Font *> fonts_;
  mutable std::array<TTF_Font *, std::numeric_limits<std::uint16_t>::max() + 1> glyph_index_cache_;
};

/* Like TTF_RenderUTF8_Shaded but supports multiple fonts. */
SDL_Surface *TTFMultiFont_RenderUTF8_Shaded(const Fonts &fonts,
                                            const std::string &text,
                                            SDL_Color fg, SDL_Color bg);

#endif  // _SDL_TTF_MULTIFONT_H_
