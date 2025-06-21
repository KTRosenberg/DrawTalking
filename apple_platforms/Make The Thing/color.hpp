//
//  color.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/23/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef color_hpp
#define color_hpp

typedef enum COLOR_SPACE {
    COLOR_SPACE_RGB,
    COLOR_SPACE_HSV,
    COLOR_SPACE_COUNT
} COLOR_SPACE;




//
// https://gist.github.com/kuathadianto/200148f53616cbd226d993b400214a7f
//
// H(Hue): 0 - 360 degree (integer)
// S(Saturation): 0 - 1.00 (double)
// V(Value): 0 - 1.00 (double)
//
// output[3]: Output, array size 3, int
//
void HSVtoRGB(int H, double S, double V, float64 output[3]);

vec4 HSVtoRGBinplace(int H, double S, double V, float64 alpha);

inline vec4 RGBint_toRGBfloat(vec4 rgbint)
{
    return vec4(rgbint.r/255.0f, rgbint.g/255.0f, rgbint.b/255.0f, rgbint.a/255.0f);
}

namespace color {

static const vec4 RED     = {1.0, 0.0, 0.0, 1.0};
static const vec4 MAGENTA = {1.0, 0.0, 1.0, 1.0};
static const vec4 YELLOW  = {1.0, 1.0, 0.0, 1.0};
static const vec4 WHITE   = {1.0, 1.0, 1.0, 1.0};
static const vec4 GREEN   = {0.0, 1.0, 0.0, 1.0};
static const vec4 CYAN    = {0.0, 1.0, 1.0, 1.0};
static const vec4 BLACK   = {0.0, 0.0, 0.0, 1.0};
static const vec4 BLUE    = {0.0, 0.0, 1.0, 1.0};
static const vec4 SYSTEM_BLUE = {RGBint_toRGBfloat(vec4(0, 181, 204, 255))};
static const vec4 CLEAR   = {0.0, 0.0, 0.0, 0.0};
static const vec4 ORANGE   = {1.0, 165.0f/255.0f, 0.0, 1.0};

static const vec4 EARTHY_1   = vec4(72.0f/255.0f,52.0f/255.0f,73.0f/255.0f,1.0f);
static const vec4 DARK_PURPLE_1 = vec4(24.0f/255.0f,10.0f/255.0f,108.0f/255.0f,1.0f);
static const vec4 REGAL_RED_1 = vec4(155.0f/255.0f,28.0f/255.0f,86.0f/255.0f,1.0f);
static const vec4 REGAL_RED_2 = vec4(122.0f/255.0f,57.0f/255.0f,100.0f/255.0f,1.0f);



struct rgba {
    rgba() : v4(static_cast<vec4>(0.0f)) { }
    rgba(vec4 val) : v4(val) { }
    operator vec4&() { return this->v4; }
    operator vec4() const { return this->v4; }
    
    rgba(const rgba& to_copy) : v4(to_copy) {}
    
    rgba(rgba&& to_move) noexcept :
    v4(std::move(to_move))
    { }
    
    rgba& operator=(rgba other)
    {
        this->v4 = other;
        return *this;
    }
    
    rgba& operator=(vec4 other)
    {
        this->v4 = other;
        return *this;
    }
    
    inline float32 operator[](const int index)
    {
        return this->v4[index];
    }
    
    inline const float32& operator[](const int index) const
    {
        return this->v4[index];
    }
    

    vec4 v4;
};

void print_bgra_buffer_as_rgba(uint8 * data, usize width, usize height);

void print_bgra_buffer_as_bgra(uint8 * data, usize width, usize height);


inline static float32 int_to_float(uint32 color_component)
{
    return ((float32)color_component) / 255.0f;
}

inline static float32 i2f(uint32 color_component)
{
    return int_to_float(color_component);
}

inline static float32 float_to_int(float32 color_component)
{
    return m::clamp((uint32)(color_component * 255), (uint32)0, (uint32)255);
}

inline static float32 f2i(float32 color_component)
{
    return float_to_int(color_component);
}

const vec4 make_with_alpha(vec4 rgba, float32 alpha);



namespace hsv49
{

static inline const std::vector<mtt::String> color_names = {
    "aliceblue",
    "antiquewhite",
    "aqua",
    "aquamarine",
    "azure",
    "beige",
    "bisque",
    "black",
    "blanchedalmond",
    "blue",
    "blueviolet",
    "brown",
    "burlywood",
    "cadetblue",
    "chartreuse",
    "chocolate",
    "coral",
    "cornflowerblue",
    "cornsilk",
    "crimson",
    "cyan",
    "darkblue",
    "darkcyan",
    "darkgoldenrod",
    "darkgray",
    "darkgreen",
    "darkgrey",
    "darkkhaki",
    "darkmagenta",
    "darkolivegreen",
    "darkorange",
    "darkorchid",
    "darkred",
    "darksalmon",
    "darkseagreen",
    "darkslateblue",
    "darkslategray",
    "darkslategrey",
    "darkturquoise",
    "darkviolet",
    "deeppink",
    "deepskyblue",
    "dimgray",
    "dimgrey",
    "dodgerblue",
    "firebrick",
    "floralwhite",
    "forestgreen",
    "fuchsia",
    "gainsboro",
    "ghostwhite",
    "goldenrod",
    "gold",
    "gray",
    "green",
    "greenyellow",
    "grey",
    "honeydew",
    "hotpink",
    "indianred",
    "indigo",
    "ivory",
    "khaki",
    "lavenderblush",
    "lavender",
    "lawngreen",
    "lemonchiffon",
    "lightblue",
    "lightcoral",
    "lightcyan",
    "lightgoldenrodyellow",
    "lightgray",
    "lightgreen",
    "lightgrey",
    "lightpink",
    "lightsalmon",
    "lightseagreen",
    "lightskyblue",
    "lightslategray",
    "lightslategrey",
    "lightsteelblue",
    "lightyellow",
    "lime",
    "limegreen",
    "linen",
    "magenta",
    "maroon",
    "mediumaquamarine",
    "mediumblue",
    "mediumorchid",
    "mediumpurple",
    "mediumseagreen",
    "mediumslateblue",
    "mediumspringgreen",
    "mediumturquoise",
    "mediumvioletred",
    "midnightblue",
    "mintcream",
    "mistyrose",
    "moccasin",
    "navajowhite",
    "navy",
    "oldlace",
    "olive",
    "olivedrab",
    "orange",
    "orangered",
    "orchid",
    "palegoldenrod",
    "palegreen",
    "paleturquoise",
    "palevioletred",
    "papayawhip",
    "peachpuff",
    "peru",
    "pink",
    "plum",
    "powderblue",
    "purple",
    "rebeccapurple",
    "red",
    "rosybrown",
    "royalblue",
    "saddlebrown",
    "salmon",
    "sandybrown",
    "seagreen",
    "seashell",
    "sienna",
    "silver",
    "skyblue",
    "slateblue",
    "slategray",
    "slategrey",
    "snow",
    "springgreen",
    "steelblue",
    "tan",
    "teal",
    "thistle",
    "tomato",
    "turquoise",
    "violet",
    "wheat",
    "white",
    "whitesmoke",
    "yellow",
    "yellowgreen"
};

static inline robin_hood::unordered_flat_map<std::string, Vector3> color_values = {
    {"aliceblue", {0.5777777777777778, 0.058823529411764705, 255}},
    {"antiquewhite", {0.09523809523809523, 0.14, 250}},
    {"aqua", {0.5, 1.0, 255}},
    {"aquamarine", {0.4440104166666667, 0.5019607843137255, 255}},
    {"azure", {0.5, 0.058823529411764705, 255}},
    {"beige", {0.16666666666666666, 0.10204081632653061, 245}},
    {"bisque", {0.0903954802259887, 0.23137254901960785, 255}},
    {"black", {0.0, 0.0, 0}},
    {"blanchedalmond", {0.09999999999999999, 0.19607843137254902, 255}},
    {"blue", {0.6666666666666666, 1.0, 255}},
    {"blueviolet", {0.7531876138433516, 0.8097345132743363, 226}},
    {"brown", {0.0, 0.7454545454545455, 165}},
    {"burlywood", {0.09386973180076628, 0.3918918918918919, 222}},
    {"cadetblue", {0.5051282051282051, 0.40625, 160}},
    {"chartreuse", {0.2503267973856209, 1.0, 255}},
    {"chocolate", {0.06944444444444443, 0.8571428571428571, 210}},
    {"coral", {0.04476190476190476, 0.6862745098039216, 255}},
    {"cornflowerblue", {0.6070559610705596, 0.5780590717299579, 237}},
    {"cornsilk", {0.13333333333333333, 0.13725490196078433, 255}},
    {"crimson", {0.9666666666666667, 0.9090909090909091, 220}},
    {"cyan", {0.5, 1.0, 255}},
    {"darkblue", {0.6666666666666666, 1.0, 139}},
    {"darkcyan", {0.5, 1.0, 139}},
    {"darkgoldenrod", {0.1184971098265896, 0.9402173913043478, 184}},
    {"darkgray", {0.0, 0.0, 169}},
    {"darkgreen", {0.3333333333333333, 1.0, 100}},
    {"darkgrey", {0.0, 0.0, 169}},
    {"darkkhaki", {0.15447154471544716, 0.43386243386243384, 189}},
    {"darkmagenta", {0.8333333333333334, 1.0, 139}},
    {"darkolivegreen", {0.22777777777777777, 0.5607476635514018, 107}},
    {"darkorange", {0.09150326797385622, 1.0, 255}},
    {"darkorchid", {0.7781385281385281, 0.7549019607843137, 204}},
    {"darkred", {0.0, 1.0, 139}},
    {"darksalmon", {0.04204204204204204, 0.47639484978540775, 233}},
    {"darkseagreen", {0.3333333333333333, 0.2393617021276596, 188}},
    {"darkslateblue", {0.6901709401709403, 0.5611510791366906, 139}},
    {"darkslategray", {0.5, 0.4050632911392405, 79}},
    {"darkslategrey", {0.5, 0.4050632911392405, 79}},
    {"darkturquoise", {0.5023923444976077, 1.0, 209}},
    {"darkviolet", {0.7835703001579778, 1.0, 211}},
    {"deeppink", {0.9099290780141844, 0.9215686274509803, 255}},
    {"deepskyblue", {0.5418300653594771, 1.0, 255}},
    {"dimgray", {0.0, 0.0, 105}},
    {"dimgrey", {0.0, 0.0, 105}},
    {"dodgerblue", {0.5822222222222222, 0.8823529411764706, 255}},
    {"firebrick", {0.0, 0.8089887640449438, 178}},
    {"floralwhite", {0.11111111111111112, 0.058823529411764705, 255}},
    {"forestgreen", {0.3333333333333333, 0.7553956834532374, 139}},
    {"fuchsia", {0.8333333333333334, 1.0, 255}},
    {"gainsboro", {0.0, 0.0, 220}},
    {"ghostwhite", {0.6666666666666666, 0.027450980392156862, 255}},
    {"goldenrod", {0.11917562724014337, 0.8532110091743119, 218}},
    {"gold", {0.14052287581699346, 1.0, 255}},
    {"gray", {0.0, 0.0, 128}},
    {"green", {0.3333333333333333, 1.0, 128}},
    {"greenyellow", {0.23237179487179485, 0.8156862745098039, 255}},
    {"grey", {0.0, 0.0, 128}},
    {"honeydew", {0.3333333333333333, 0.058823529411764705, 255}},
    {"hotpink", {0.9166666666666666, 0.5882352941176471, 255}},
    {"indianred", {0.0, 0.551219512195122, 205}},
    {"indigo", {0.7628205128205128, 1.0, 130}},
    {"ivory", {0.16666666666666666, 0.058823529411764705, 255}},
    {"khaki", {0.15, 0.4166666666666667, 240}},
    {"lavenderblush", {0.9444444444444444, 0.058823529411764705, 255}},
    {"lavender", {0.6666666666666666, 0.08, 250}},
    {"lawngreen", {0.25132275132275134, 1.0, 252}},
    {"lemonchiffon", {0.15, 0.19607843137254902, 255}},
    {"lightblue", {0.5409356725146198, 0.24782608695652175, 230}},
    {"lightcoral", {0.0, 0.4666666666666667, 240}},
    {"lightcyan", {0.5, 0.12156862745098039, 255}},
    {"lightgoldenrodyellow", {0.16666666666666666, 0.16, 250}},
    {"lightgray", {0.0, 0.0, 211}},
    {"lightgreen", {0.3333333333333333, 0.3949579831932773, 238}},
    {"lightgrey", {0.0, 0.0, 211}},
    {"lightpink", {0.9748858447488584, 0.28627450980392155, 255}},
    {"lightsalmon", {0.047619047619047616, 0.5215686274509804, 255}},
    {"lightseagreen", {0.49086757990867574, 0.8202247191011236, 178}},
    {"lightskyblue", {0.5637681159420289, 0.46, 250}},
    {"lightslategray", {0.5833333333333334, 0.2222222222222222, 153}},
    {"lightslategrey", {0.5833333333333334, 0.2222222222222222, 153}},
    {"lightsteelblue", {0.5942028985507246, 0.2072072072072072, 222}},
    {"lightyellow", {0.16666666666666666, 0.12156862745098039, 255}},
    {"lime", {0.3333333333333333, 1.0, 255}},
    {"limegreen", {0.3333333333333333, 0.7560975609756098, 205}},
    {"linen", {0.08333333333333333, 0.08, 250}},
    {"magenta", {0.8333333333333334, 1.0, 255}},
    {"maroon", {0.0, 1.0, 128}},
    {"mediumaquamarine", {0.44336569579288027, 0.5024390243902439, 205}},
    {"mediumblue", {0.6666666666666666, 1.0, 205}},
    {"mediumorchid", {0.8002645502645502, 0.5971563981042654, 211}},
    {"mediumpurple", {0.721183800623053, 0.4885844748858447, 219}},
    {"mediumseagreen", {0.4075630252100841, 0.664804469273743, 179}},
    {"mediumslateblue", {0.6902985074626865, 0.5630252100840336, 238}},
    {"mediumspringgreen", {0.436, 1.0, 250}},
    {"mediumturquoise", {0.49391727493917276, 0.6555023923444976, 209}},
    {"mediumvioletred", {0.8951310861423221, 0.8944723618090452, 199}},
    {"midnightblue", {0.6666666666666666, 0.7767857142857143, 112}},
    {"mintcream", {0.4166666666666667, 0.0392156862745098, 255}},
    {"mistyrose", {0.016666666666666663, 0.11764705882352941, 255}},
    {"moccasin", {0.10585585585585584, 0.2901960784313726, 255}},
    {"navajowhite", {0.09959349593495935, 0.3215686274509804, 255}},
    {"navy", {0.6666666666666666, 1.0, 128}},
    {"oldlace", {0.10869565217391304, 0.09090909090909091, 253}},
    {"olive", {0.16666666666666666, 1.0, 128}},
    {"olivedrab", {0.22118380062305296, 0.7535211267605634, 142}},
    {"orange", {0.10784313725490195, 1.0, 255}},
    {"orangered", {0.04509803921568628, 1.0, 255}},
    {"orchid", {0.839622641509434, 0.48623853211009177, 218}},
    {"palegoldenrod", {0.15196078431372548, 0.2857142857142857, 238}},
    {"palegreen", {0.3333333333333333, 0.3944223107569721, 251}},
    {"paleturquoise", {0.5, 0.2647058823529412, 238}},
    {"palevioletred", {0.9454828660436136, 0.4885844748858447, 219}},
    {"papayawhip", {0.10317460317460318, 0.16470588235294117, 255}},
    {"peachpuff", {0.07857142857142857, 0.27450980392156865, 255}},
    {"peru", {0.08215962441314555, 0.6926829268292682, 205}},
    {"pink", {0.9708994708994709, 0.24705882352941178, 255}},
    {"plum", {0.8333333333333334, 0.27601809954751133, 221}},
    {"powderblue", {0.5185185185185185, 0.23478260869565218, 230}},
    {"purple", {0.8333333333333334, 1.0, 128}},
    {"rebeccapurple", {0.75, 0.6666666666666666, 153}},
    {"red", {0.0, 1.0, 255}},
    {"rosybrown", {0.0, 0.2393617021276596, 188}},
    {"royalblue", {0.625, 0.7111111111111111, 225}},
    {"saddlebrown", {0.06944444444444443, 0.8633093525179856, 139}},
    {"salmon", {0.017156862745098034, 0.544, 250}},
    {"sandybrown", {0.07657657657657657, 0.6065573770491803, 244}},
    {"seagreen", {0.4068100358422939, 0.6690647482014388, 139}},
    {"seashell", {0.06862745098039215, 0.06666666666666667, 255}},
    {"sienna", {0.053623188405797106, 0.71875, 160}},
    {"silver", {0.0, 0.0, 192}},
    {"skyblue", {0.5483333333333333, 0.425531914893617, 235}},
    {"slateblue", {0.6898550724637681, 0.5609756097560976, 205}},
    {"slategray", {0.5833333333333334, 0.2222222222222222, 144}},
    {"slategrey", {0.5833333333333334, 0.2222222222222222, 144}},
    {"snow", {0.0, 0.0196078431372549, 255}},
    {"springgreen", {0.41633986928104577, 1.0, 255}},
    {"steelblue", {0.5757575757575757, 0.6111111111111112, 180}},
    {"tan", {0.09523809523809523, 0.3333333333333333, 210}},
    {"teal", {0.5, 1.0, 128}},
    {"thistle", {0.8333333333333334, 0.11574074074074074, 216}},
    {"tomato", {0.025362318840579712, 0.7215686274509804, 255}},
    {"turquoise", {0.48333333333333334, 0.7142857142857143, 224}},
    {"violet", {0.8333333333333334, 0.453781512605042, 238}},
    {"wheat", {0.10858585858585858, 0.2693877551020408, 245}},
    {"white", {0.0, 0.0, 255}},
    {"whitesmoke", {0.0, 0.0, 245}},
    {"yellow", {0.16666666666666666, 1.0, 255}},
    {"yellowgreen", {0.5315091210613598, 0.9571428571428572, 210}}
};

static inline usize strdiff(const std::string& source, const std::string& target)
{
    const usize n = source.length();
    const usize m = target.length();
    if (n == 0)
    {
        return m;
    }
    if (m == 0)
    {
        return n;
    }
    static std::vector<std::vector<int> > matrix(n + 1);
    matrix.resize(n + 1);
    for (int i = 0; i <= n; i++)
    {
        matrix[i].resize(m + 1);
    }
    for (int i = 0; i <= n; i++)
    {
        matrix[i][0] = i;
    }
    for (int j = 0; j <= m; j++)
    {
        matrix[0][j] = j;
    }
    for (int i = 1; i <= n; i++)
    {
        const char s_i = source[i - 1];
        for (int j = 1; j <= m; j++)
        {
            const char t_j = target[j - 1];
            int cost;
            if (s_i == t_j)
            {
                cost = 0;
            }
            else
            {
                cost = 1;
            }
            const int above = matrix[i - 1][j];
            const int left = matrix[i][j - 1];
            const int diag = matrix[i - 1][j - 1];
            int cell = std::min(above + 1, std::min(left + 1, diag + cost));
            if (i > 2 && j > 2)
            {
                int trans = matrix[i - 2][j - 2] + 1;
                if (source[i - 2] != t_j)
                    trans++;
                if (s_i != target[j - 2])
                    trans++;
                if (cell > trans)
                    cell = trans;
            }
            matrix[i][j] = cell;
        }
    }
    return matrix[n][m];
}

static inline std::vector<std::string> best_names_for(const std::string& colorname)
{
    std::vector<std::string> names = color_names;
    std::sort(names.begin(), names.end(), [&](std::string& first, std::string& second)
              { return strdiff(colorname, first) < strdiff(colorname, second); });
    return names;
}

static inline Vector3 best_for(const std::string& colorname)
{
    return color_values[best_names_for(colorname)[0]];
}

}


}

#endif /* color_hpp */
