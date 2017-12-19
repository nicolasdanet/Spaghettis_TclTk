
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_CORE_COLORS_HPP
#define BELLE_CORE_COLORS_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

using namespace prim;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Color {

friend class Colors;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Color() : r_ (0.0f), g_ (0.0f), b_ (0.0f), alpha_ (1.0f) 
    {
    }
    
    Color (float r, float g, float b, float a) : r_ (r), g_ (g), b_ (b), alpha_ (a) 
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Color (const Color&) = default;
    Color (Color&&) = default;
    Color& operator = (const Color&) = default;
    Color& operator = (Color&&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    float getRed() const
    {
        return r_;
    }
    
    float getGreen() const
    {
        return g_;
    }
    
    float getBlue() const
    {
        return b_;
    }
    
    float getAlpha() const
    {
        return alpha_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Color (int32 r, int32 g, int32 b) : r_ (r / 255.0f), g_ (g / 255.0f), b_ (b / 255.0f), alpha_ (1.0f)
    {
    }
    
private:
    float r_;
    float g_;
    float b_;
    float alpha_;

private:
    PRIM_LEAK_DETECTOR (Color)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.w3.org/TR/SVG/types.html#ColorKeywords > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Colors {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static const Color empty()
    {
        return Color (0.0f, 0.0f, 0.0f, 0.0f);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    static const Color aliceblue;
    static const Color antiquewhite;
    static const Color aqua;
    static const Color aquamarine;
    static const Color azure;
    static const Color beige;
    static const Color bisque;
    static const Color black;
    static const Color blanchedalmond;
    static const Color blue;
    static const Color blueviolet;
    static const Color brown;
    static const Color burlywood;
    static const Color cadetblue;
    static const Color chartreuse;
    static const Color chocolate;
    static const Color coral;
    static const Color cornflowerblue;
    static const Color cornsilk;
    static const Color crimson;
    static const Color cyan;
    static const Color darkblue;
    static const Color darkcyan;
    static const Color darkgoldenrod;
    static const Color darkgray;
    static const Color darkgreen;
    static const Color darkgrey;
    static const Color darkkhaki;
    static const Color darkmagenta;
    static const Color darkolivegreen;
    static const Color darkorange;
    static const Color darkorchid;
    static const Color darkred;
    static const Color darksalmon;
    static const Color darkseagreen;
    static const Color darkslateblue;
    static const Color darkslategray;
    static const Color darkslategrey;
    static const Color darkturquoise;
    static const Color darkviolet;
    static const Color deeppink;
    static const Color deepskyblue;
    static const Color dimgray;
    static const Color dimgrey;
    static const Color dodgerblue;
    static const Color firebrick;
    static const Color floralwhite;
    static const Color forestgreen;
    static const Color fuchsia;
    static const Color gainsboro;
    static const Color ghostwhite;
    static const Color gold;
    static const Color goldenrod;
    static const Color gray;
    static const Color grey;
    static const Color green;
    static const Color greenyellow;
    static const Color honeydew;
    static const Color hotpink;
    static const Color indianred;
    static const Color indigo;
    static const Color ivory;
    static const Color khaki;
    static const Color lavender;
    static const Color lavenderblush;
    static const Color lawngreen;
    static const Color lemonchiffon;
    static const Color lightblue;
    static const Color lightcoral;
    static const Color lightcyan;
    static const Color lightgoldenrodyellow;
    static const Color lightgray;
    static const Color lightgreen;
    static const Color lightgrey;
    static const Color lightpink;
    static const Color lightsalmon;
    static const Color lightseagreen;
    static const Color lightskyblue;
    static const Color lightslategray;
    static const Color lightslategrey;
    static const Color lightsteelblue;
    static const Color lightyellow;
    static const Color lime;
    static const Color limegreen;
    static const Color linen;
    static const Color magenta;
    static const Color maroon;
    static const Color mediumaquamarine;
    static const Color mediumblue;
    static const Color mediumorchid;
    static const Color mediumpurple;
    static const Color mediumseagreen;
    static const Color mediumslateblue;
    static const Color mediumspringgreen;
    static const Color mediumturquoise;
    static const Color mediumvioletred;
    static const Color midnightblue;
    static const Color mintcream;
    static const Color mistyrose;
    static const Color moccasin;
    static const Color navajowhite;
    static const Color navy;
    static const Color oldlace;
    static const Color olive;
    static const Color olivedrab;
    static const Color orange;
    static const Color orangered;
    static const Color orchid;
    static const Color palegoldenrod;
    static const Color palegreen;
    static const Color paleturquoise;
    static const Color palevioletred;
    static const Color papayawhip;
    static const Color peachpuff;
    static const Color peru;
    static const Color pink;
    static const Color plum;
    static const Color powderblue;
    static const Color purple;
    static const Color red;
    static const Color rosybrown;
    static const Color royalblue;
    static const Color saddlebrown;
    static const Color salmon;
    static const Color sandybrown;
    static const Color seagreen;
    static const Color seashell;
    static const Color sienna;
    static const Color silver;
    static const Color skyblue;
    static const Color slateblue;
    static const Color slategray;
    static const Color slategrey;
    static const Color snow;
    static const Color springgreen;
    static const Color steelblue;
    static const Color tan;
    static const Color teal;
    static const Color thistle;
    static const Color tomato;
    static const Color turquoise;
    static const Color violet;
    static const Color wheat;
    static const Color white;
    static const Color whitesmoke;
    static const Color yellow;
    static const Color yellowgreen;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

const Color Colors::aliceblue               (240, 248, 255);
const Color Colors::antiquewhite            (250, 235, 215);
const Color Colors::aqua                    (0,   255, 255);
const Color Colors::aquamarine              (127, 255, 212);
const Color Colors::azure                   (240, 255, 255);
const Color Colors::beige                   (245, 245, 220);
const Color Colors::bisque                  (255, 228, 196);
const Color Colors::black                   (0,   0,   0);
const Color Colors::blanchedalmond          (255, 235, 205);
const Color Colors::blue                    (0,   0,   255);
const Color Colors::blueviolet              (138, 43,  226);
const Color Colors::brown                   (165, 42,  42);
const Color Colors::burlywood               (222, 184, 135);
const Color Colors::cadetblue               (95,  158, 160);
const Color Colors::chartreuse              (127, 255, 0);
const Color Colors::chocolate               (210, 105, 30);
const Color Colors::coral                   (255, 127, 80);
const Color Colors::cornflowerblue          (100, 149, 237);
const Color Colors::cornsilk                (255, 248, 220);
const Color Colors::crimson                 (220, 20,  60);
const Color Colors::cyan                    (0,   255, 255);
const Color Colors::darkblue                (0,   0,   139);
const Color Colors::darkcyan                (0,   139, 139);
const Color Colors::darkgoldenrod           (184, 134, 11);
const Color Colors::darkgray                (169, 169, 169);
const Color Colors::darkgreen               (0,   100, 0);
const Color Colors::darkgrey                (169, 169, 169);
const Color Colors::darkkhaki               (189, 183, 107);
const Color Colors::darkmagenta             (139, 0,   139);
const Color Colors::darkolivegreen          (85,  107, 47);
const Color Colors::darkorange              (255, 140, 0);
const Color Colors::darkorchid              (153, 50,  204);
const Color Colors::darkred                 (139, 0,   0);
const Color Colors::darksalmon              (233, 150, 122);
const Color Colors::darkseagreen            (143, 188, 143);
const Color Colors::darkslateblue           (72,  61,  139);
const Color Colors::darkslategray           (47,  79,  79);
const Color Colors::darkslategrey           (47,  79,  79);
const Color Colors::darkturquoise           (0,   206, 209);
const Color Colors::darkviolet              (148, 0,   211);
const Color Colors::deeppink                (255, 20,  147);
const Color Colors::deepskyblue             (0,   191, 255);
const Color Colors::dimgray                 (105, 105, 105);
const Color Colors::dimgrey                 (105, 105, 105);
const Color Colors::dodgerblue              (30,  144, 255);
const Color Colors::firebrick               (178, 34,  34);
const Color Colors::floralwhite             (255, 250, 240);
const Color Colors::forestgreen             (34,  139, 34);
const Color Colors::fuchsia                 (255, 0,   255);
const Color Colors::gainsboro               (220, 220, 220);
const Color Colors::ghostwhite              (248, 248, 255);
const Color Colors::gold                    (255, 215, 0);
const Color Colors::goldenrod               (218, 165, 32);
const Color Colors::gray                    (128, 128, 128);
const Color Colors::grey                    (128, 128, 128);
const Color Colors::green                   (0,   128, 0);
const Color Colors::greenyellow             (173, 255, 47);
const Color Colors::honeydew                (240, 255, 240);
const Color Colors::hotpink                 (255, 105, 180);
const Color Colors::indianred               (205, 92,  92);
const Color Colors::indigo                  (75,  0,   130);
const Color Colors::ivory                   (255, 255, 240);
const Color Colors::khaki                   (240, 230, 140);
const Color Colors::lavender                (230, 230, 250);
const Color Colors::lavenderblush           (255, 240, 245);
const Color Colors::lawngreen               (124, 252, 0);
const Color Colors::lemonchiffon            (255, 250, 205);
const Color Colors::lightblue               (173, 216, 230);
const Color Colors::lightcoral              (240, 128, 128);
const Color Colors::lightcyan               (224, 255, 255);
const Color Colors::lightgoldenrodyellow    (250, 250, 210);
const Color Colors::lightgray               (211, 211, 211);
const Color Colors::lightgreen              (144, 238, 144);
const Color Colors::lightgrey               (211, 211, 211);
const Color Colors::lightpink               (255, 182, 193);
const Color Colors::lightsalmon             (255, 160, 122);
const Color Colors::lightseagreen           (32,  178, 170);
const Color Colors::lightskyblue            (135, 206, 250);
const Color Colors::lightslategray          (119, 136, 153);
const Color Colors::lightslategrey          (119, 136, 153);
const Color Colors::lightsteelblue          (176, 196, 222);
const Color Colors::lightyellow             (255, 255, 224);
const Color Colors::lime                    (0,   255, 0);
const Color Colors::limegreen               (50,  205, 50);
const Color Colors::linen                   (250, 240, 230);
const Color Colors::magenta                 (255, 0,   255);
const Color Colors::maroon                  (128, 0,   0);
const Color Colors::mediumaquamarine        (102, 205, 170);
const Color Colors::mediumblue              (0,   0,   205);
const Color Colors::mediumorchid            (186, 85,  211);
const Color Colors::mediumpurple            (147, 112, 219);
const Color Colors::mediumseagreen          (60,  179, 113);
const Color Colors::mediumslateblue         (123, 104, 238);
const Color Colors::mediumspringgreen       (0,   250, 154);
const Color Colors::mediumturquoise         (72,  209, 204);
const Color Colors::mediumvioletred         (199, 21,  133);
const Color Colors::midnightblue            (25,  25,  112);
const Color Colors::mintcream               (245, 255, 250);
const Color Colors::mistyrose               (255, 228, 225);
const Color Colors::moccasin                (255, 228, 181);
const Color Colors::navajowhite             (255, 222, 173);
const Color Colors::navy                    (0,   0,   128);
const Color Colors::oldlace                 (253, 245, 230);
const Color Colors::olive                   (128, 128, 0);
const Color Colors::olivedrab               (107, 142, 35);
const Color Colors::orange                  (255, 165, 0);
const Color Colors::orangered               (255, 69,  0);
const Color Colors::orchid                  (218, 112, 214);
const Color Colors::palegoldenrod           (238, 232, 170);
const Color Colors::palegreen               (152, 251, 152);
const Color Colors::paleturquoise           (175, 238, 238);
const Color Colors::palevioletred           (219, 112, 147);
const Color Colors::papayawhip              (255, 239, 213);
const Color Colors::peachpuff               (255, 218, 185);
const Color Colors::peru                    (205, 133, 63);
const Color Colors::pink                    (255, 192, 203);
const Color Colors::plum                    (221, 160, 221);
const Color Colors::powderblue              (176, 224, 230);
const Color Colors::purple                  (128, 0,   128);
const Color Colors::red                     (255, 0,   0);
const Color Colors::rosybrown               (188, 143, 143);
const Color Colors::royalblue               (65,  105, 225);
const Color Colors::saddlebrown             (139, 69,  19);
const Color Colors::salmon                  (250, 128, 114);
const Color Colors::sandybrown              (244, 164, 96);
const Color Colors::seagreen                (46,  139, 87);
const Color Colors::seashell                (255, 245, 238);
const Color Colors::sienna                  (160, 82,  45);
const Color Colors::silver                  (192, 192, 192);
const Color Colors::skyblue                 (135, 206, 235);
const Color Colors::slateblue               (106, 90,  205);
const Color Colors::slategray               (112, 128, 144);
const Color Colors::slategrey               (112, 128, 144);
const Color Colors::snow                    (255, 250, 250);
const Color Colors::springgreen             (0,   255, 127);
const Color Colors::steelblue               (70,  130, 180);
const Color Colors::tan                     (210, 180, 140);
const Color Colors::teal                    (0,   128, 128);
const Color Colors::thistle                 (216, 191, 216);
const Color Colors::tomato                  (255, 99,  71);
const Color Colors::turquoise               (64,  224, 208);
const Color Colors::violet                  (238, 130, 238);
const Color Colors::wheat                   (245, 222, 179);
const Color Colors::white                   (255, 255, 255);
const Color Colors::whitesmoke              (245, 245, 245);
const Color Colors::yellow                  (255, 255, 0);
const Color Colors::yellowgreen             (154, 205, 50);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_CORE_COLORS_HPP
