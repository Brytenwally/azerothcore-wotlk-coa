#ifndef XP_HOTSPOT_PAINTING_H
#define XP_HOTSPOT_PAINTING_H

#include "XPHotspotRules.h"
#include <boost/geometry.hpp>

namespace XPHotspots
{
inline bool Paint(Location& row, Point from, Point to, double radius, bool square)
{
    namespace bg = boost::geometry;
    using XY = bg::model::d2::point_xy<double>;
    using Polygon = bg::model::polygon<XY>;
    if (row.Kind != "polygon" || !std::isfinite(radius) || radius < 5 || radius > 1000 ||
        !std::isfinite(from.X) || !std::isfinite(from.Y) || !std::isfinite(to.X) || !std::isfinite(to.Y) ||
        std::abs(from.X) > 19000 || std::abs(from.Y) > 19000 ||
        std::abs(to.X) > 19000 || std::abs(to.Y) > 19000 ||
        std::hypot(to.X - from.X, to.Y - from.Y) > 5000)
        return false;
    bg::model::multi_point<XY> samples;
    for (Point center : {from, to})
        for (size_t i = 0; i < (square ? 4u : 24u); ++i)
        {
            double angle = 6.283185307179586 * i / (square ? 4 : 24) + (square ? 0.7853981633974483 : 0);
            double reach = radius * (square ? std::sqrt(2.0) : 1);
            samples.emplace_back(center.X + reach * std::cos(angle), center.Y + reach * std::sin(angle));
        }
    Polygon stroke;
    bg::convex_hull(samples, stroke);
    Polygon result = stroke;
    if (row.Points.size() >= 3)
    {
        if (!ValidPolygon(row.Points))
            return false;
        Polygon original;
        for (Point point : row.Points)
            original.outer().emplace_back(point.X, point.Y);
        bg::correct(original);
        std::vector<Polygon> joined;
        bg::union_(original, stroke, joined);
        if (joined.size() != 1 || !joined.front().inners().empty())
            return false;
        result = joined.front();
    }
    if (result.outer().size() > MaxPolygonPoints + 1)
    {
        Polygon reduced;
        bg::simplify(result, reduced, std::min(2.0, radius / 20));
        if (!bg::is_valid(reduced) || !bg::covered_by(result, reduced))
            return false;
        result = reduced;
    }
    std::vector<Point> points;
    for (size_t i = 0; i + 1 < result.outer().size(); ++i)
        points.push_back({bg::get<0>(result.outer()[i]), bg::get<1>(result.outer()[i])});
    if (!ValidPolygon(points))
        return false;
    if (row.Points.size() < 3)
        row.Center = from;
    row.Points = std::move(points);
    row.Source = "gm-paint";
    return true;
}

inline bool ChangeShape(Location& row, std::string const& kind)
{
    if ((kind != "circle" && kind != "polygon") || (row.Kind != "circle" && row.Kind != "polygon"))
        return false;
    if (kind == row.Kind)
        return true;
    if (kind == "circle")
    {
        if (!ValidPolygon(row.Points))
            return false;
        Point center;
        for (Point point : row.Points)
        {
            center.X += point.X / row.Points.size();
            center.Y += point.Y / row.Points.size();
        }
        double radius = 1;
        for (Point point : row.Points)
            radius = std::max(radius, std::hypot(point.X - center.X, point.Y - center.Y));
        if (radius > 5000)
            return false;
        row.Center = center;
        row.Radius = radius;
        row.Points.clear();
    }
    else
    {
        row.Points.clear();
        for (size_t i = 0; i < 24; ++i)
        {
            double angle = 6.283185307179586 * i / 24;
            row.Points.push_back({row.Center.X + row.Radius * std::cos(angle),
                row.Center.Y + row.Radius * std::sin(angle)});
        }
        if (!ValidPolygon(row.Points))
            return false;
    }
    row.Kind = kind;
    row.Source = "gm-shape";
    return true;
}
}

#endif
