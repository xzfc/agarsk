#include <fstream>
struct Svg {
  Svg(const char *filename, Aabb cords, unsigned w, unsigned h) : f(filename) {
    f << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='" << cords.x0 << " "
      << cords.y0 << " " << cords.x1 - cords.x0 << " " << cords.y1 - cords.y0
      << "' width='" << w << "' height='" << h << "' version='1.0'>\n";
  }

  ~Svg() { f << "</svg>\n"; }

  void circle(double x, double y, double r, const char *stroke,
              const char *fill) {
    f << "  <circle "
         "r='" << r << "' "
                       "cx='" << x << "' "
                                      "cy='" << y << "' "
                                                     "stroke='" << stroke
      << "' "
         "stroke-width='0.25' "
         "fill='" << fill << "'/>\n";
  }

  void rectangle(Aabb rect, const char *stroke, const char *fill) {
    f << "  <rect "
         "x='" << rect.x0 << "' "
                             "y='" << rect.y0 << "' "
                                                 "width='" << rect.x1 - rect.x0
      << "' "
         "height='" << rect.y1 - rect.y0 << "' "
                                            "stroke='" << stroke
      << "' "
         "stroke-width='0.25' "
         "fill='" << fill << "'/>\n";
  }

  void line(Vec2 from, Vec2 to, const char *stroke) {
    f << "  <line "
         "x1='" << from.x << "' "
                             "y1='" << from.y << "' "
                                                 "x2='" << to.x << "' "
                                                                   "y2='"
      << to.y << "' "
                 "style='stroke:" << stroke << ";stroke-width:0.25'/>";
  }

  std::ofstream f;
};
