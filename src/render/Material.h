#pragma once

#include "ARGBColor.h"
#include "FastMat.h"
#include "LevelLoader.h"
#include "Parser.h"

#include <algorithm>
#include <cmath>

#define MAX_SHININESS_EXP 1024.0f

class Material {
public:
    void* operator new(std::size_t) = delete;
    bool operator==(const Material& other) { return color == other.color && specular == other.specular; }
    bool operator!=(const Material& other) { return color != other.color && specular != other.specular; }
    constexpr Material(ARGBColor color = 0xFF000000, ARGBColor specular = 0x00000000): color(color), specular(specular) {}
    inline ARGBColor GetColor() const { return color; }
    inline ARGBColor GetSpecular() const { return specular.WithA(0x00); }
    inline bool HasAlpha() const { return color.HasAlpha(); }
    inline Material WithColor(ARGBColor c) const {
        return Material(c, specular);
    }
    inline Material WithSpecular(ARGBColor s) const {
        return Material(color, s.WithA(specular.GetA()));
    }
    
    // Convenience getters.
    inline uint8_t GetA() const { return color.GetA(); }
    inline uint8_t GetR() const { return color.GetR(); }
    inline uint8_t GetG() const { return color.GetG(); }
    inline uint8_t GetB() const { return color.GetB(); }
    inline uint8_t GetSpecR() const { return specular.GetR(); }
    inline uint8_t GetSpecG() const { return specular.GetG(); }
    inline uint8_t GetSpecB() const { return specular.GetB(); }
    inline uint8_t GetShininess() const { return specular.GetA(); }
    
    // Convenience setters.
    inline Material WithA(uint8_t a) const {
        return Material(color.WithA(a), specular);
    }
    inline Material WithR(uint8_t r) const {
        return Material(color.WithR(r), specular);
    }
    inline Material WithG(uint8_t g) const {
        return Material(color.WithG(g), specular);
    }
    inline Material WithB(uint8_t b) const {
        return Material(color.WithB(b), specular);
    }
    inline Material WithSpecR(uint8_t r) const {
        return Material(color, specular.WithR(r));
    }
    inline Material WithSpecG(uint8_t g) const {
        return Material(color, specular.WithG(g));
    }
    inline Material WithSpecB(uint8_t b) const {
        return Material(color, specular.WithB(b));
    }
    inline Material WithShininess(uint8_t s) const {
        return Material(color, specular.WithA(s));
    }
    inline Material WithShininessVar(short index) {
        Fixed shininess = ReadFixedMaterialVar(index);
        return Material(color, specular.WithA(ConstrainShininess(shininess)));
    }
    inline Material WithShininessVar(const char *s) {
        Fixed shininess = ReadFixedMaterialVar(s);
        return Material(color, specular.WithA(ConstrainShininess(shininess)));
    }
private:
    ARGBColor color = 0xFF000000;
    ARGBColor specular = 0x00000000; // Alpha channel = shininess value
    
    /**
     * Constrain input values for shininess.
     *
     * @param shininess  The fixed-point shininess value. Valid range is 0 to MAX_SHININESS_EXP.
     * @return the nearest approximate value that we can cram into 8 bits, i.e. scaled to a range of 0 - 255
     */
    inline uint8_t ConstrainShininess(Fixed shininess) {
        return std::round(std::clamp(ToFloat(shininess), 0.0f, MAX_SHININESS_EXP) * 255 / MAX_SHININESS_EXP);
    }
};
