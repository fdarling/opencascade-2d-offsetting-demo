#include "import_shape.h"

#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>

static bool attempt_finishing_wire(outline_with_holes_vector & outlines_with_holes, BRepBuilderAPI_MakeWire &wire_builder, std::vector<string_vector>::size_type line)
{
    // try to complete the last outline or hole
    if (!outlines_with_holes.empty())
    {
        // make sure we have a valid wire to use
        if (!wire_builder.IsDone())
        {
            printf("wire_builder.IsDone() failed after processing line %zu\n", line);
            return false;
        }

        printf("wire_builder.Wire().Orientation() = %i\n", wire_builder.Wire().Orientation());
        // store the outline

        TopoDS_Wire new_wire = wire_builder.Wire();

        if (outlines_with_holes.back().first.IsNull())
        {
            if (new_wire.Orientation() != TopAbs_FORWARD)
            {
                new_wire.Reverse();
            }
            outlines_with_holes.back().first = new_wire;
        }
        else // otherwise store the last hole
        {
            if (new_wire.Orientation() != TopAbs_REVERSED)
            {
                new_wire.Reverse();
            }
            outlines_with_holes.back().second.push_back(new_wire);
        }
        wire_builder = BRepBuilderAPI_MakeWire(); // TODO is there a better way of resetting it?
    }
    return true;
}

bool collect_segments_arcs_to_wires(outline_with_holes_vector & outlines_with_holes, const std::vector<string_vector> & lines)
{
    BRepBuilderAPI_MakeWire wire_builder;
    std::vector<string_vector>::size_type line = 0;
    for (; line < lines.size(); ++line)
    {
        const string_vector &words = lines[line];
        if (words[0] == "segment")
        {
            if (words.size() != 5)
            {
                printf("line %zu with segment data must contain 5 entries, but only %lu found.\n", line, words.size());
                return false;
            }

            double pts[4];
            for (size_t j = 1; j < 5; ++j)
            {
                if (sscanf(words[j].c_str(), "%lf", &pts[j - 1]) != 1)
                {
                    printf("line %zu with segment data has invalid number in field %zu\n", line, j);
                    return false;
                }
            }
            TopoDS_Vertex vtxs[2] = {BRepBuilderAPI_MakeVertex(gp_Pnt(pts[0], pts[1], 0)), BRepBuilderAPI_MakeVertex(gp_Pnt(pts[2], pts[3], 0))};
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(vtxs[0], vtxs[1]);
            wire_builder.Add(edge);
            // segment_count++;
        }
        else
        if (words[0] == "arc" || words[0] == "arc_degrees")
        {
            if (words.size() != 6)
            {
                printf("line %zu with arc data must contain 6 entries, but only %lu found.\n", line, words.size());
                return false;
            }
            // c_x c_y R angle_start angle_end
            double arc_data[5];
            for (size_t j = 1; j < 6; ++j)
            {
                if (sscanf(words[j].c_str(), "%lf", &arc_data[j - 1]) != 1)
                {
                    printf("line %zu with arc data has invalid number in field %zu\n", line, j);
                    return false;
                }
            }
            bool full_circle = false;

            if (words[0] == "arc_degrees")
            {
                if (std::abs(arc_data[3] - arc_data[4]) == 360)
                {
                    full_circle = true;
                }
                else
                {
                    arc_data[3] *= (M_PI / 180.0);
                    arc_data[4] *= (M_PI / 180.0);
                }
            }
            else
            {
                if (std::abs(arc_data[3] - arc_data[4]) == 2 * M_PI)
                {
                    full_circle = true;
                }
            }

            if (arc_data[3] == arc_data[4])
            {
                full_circle = true;
            }

            gp_Pnt center(arc_data[0], arc_data[1], 0);
            gp_Pnt p0(center), p1(center), pm(center);

            TopoDS_Edge edge;
            if (!full_circle)
            {
                double mid_angle = 0.5 * (arc_data[3] + arc_data[4]);
                if (arc_data[3] > arc_data[4])
                {
                    mid_angle += M_PI;
                }

                p0.SetX(p0.X() + arc_data[2] * cos(arc_data[3]));
                p0.SetY(p0.Y() + arc_data[2] * sin(arc_data[3]));

                p1.SetX(p1.X() + arc_data[2] * cos(arc_data[4]));
                p1.SetY(p1.Y() + arc_data[2] * sin(arc_data[4]));


                pm.SetX(pm.X() + arc_data[2] * cos(mid_angle));
                pm.SetY(pm.Y() + arc_data[2] * sin(mid_angle));

                Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(p0, pm, p1);
                edge = BRepBuilderAPI_MakeEdge(arc);
            }
            else
            {
                p0.SetX(p0.X() - arc_data[2]);
                p1.SetX(p1.X() + arc_data[2]);
                pm.SetY(pm.Y() + arc_data[2]);

                Handle(Geom_Circle) arc = GC_MakeCircle(p0, pm, p1).Value();
                edge = BRepBuilderAPI_MakeEdge(arc);
            }

            wire_builder.Add(edge);
            // segment_count++;
        }
        else
        if (words[0] == "border" || words[0] == "hole")
        {
            if (!attempt_finishing_wire(outlines_with_holes, wire_builder, line))
            {
                printf("couldn't finish border/hole before starting another\n");
                return false;
            }

            if (words[0] == "border")
            {
                // add a new (invalid/incomplete) outline in the vector
                outlines_with_holes.push_back(outline_with_holes());
            }
            else // if (words[0] == "hole")
            {
                if (outlines_with_holes.empty())
                {
                    printf("attempting to add a hole without first specifying a border on line %zu\n", line);
                    return false;
                }

                // NOTE: we already checked if the outline (border) was valid in the first if block

                // do nothing, we don't put invalid placeholders that will
                // be overwritten for holes, we just append them later

                // TODO set a flag so that we know if we never added any segments to the hole
            }
        }
    }

    if (!attempt_finishing_wire(outlines_with_holes, wire_builder, line))
    {
        printf("couldn't finish last border/hole\n");
    }
    return true;
}

bool load_face_from(const char * path, TopoDS_Shape & res)
{
    std::vector<string_vector> lines;
    if (!read_input_file(path, lines))
    {
        printf("FATAL ERROR: can not open input file %s\n", path);
        return false;
    }

    if (!lines.size())
    {
        printf("FATAL ERROR: file %s is empty\n", path);
        return false;
    }

    outline_with_holes_vector borders_and_holes;

    if (!collect_segments_arcs_to_wires(borders_and_holes, lines))
    {
        printf("FATAL ERROR: collect_segments_arcs_to_wires invalid data in input file %s\n", path);
        return false;
    }

    BRepBuilderAPI_MakeFace builder(gp_Pln(), borders_and_holes[0].first, true);

    for (wire_vector::const_iterator it = borders_and_holes[0].second.begin(); it != borders_and_holes[0].second.end(); ++it)
    {
        builder.Add(*it);
    }
    if (!builder.IsDone())
    {
        printf("FATAL ERROR: cBRepBuilderAPI_MakeFace.isDone input file %s\n", path);
        return false;
    }
    res = builder.Face();
    return true;
}

bool read_input_file(const char * path, std::vector<string_vector> & res)
{
    res.clear();
    std::ifstream file(path);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (!line.length())
            {
                continue;
            }
            res.push_back(string_vector());
            size_t prev = 0, pos = 0;
            do
            {
                pos = line.find(" ", prev);
                if (pos == std::string::npos)
                {
                    pos = line.length();
                }
                std::string token = line.substr(prev, pos - prev);
                if (!token.empty())
                {
                    res.back().push_back(token);
                }
                prev = pos + 1;
            }
            while (pos < line.length() && prev < line.length());
        }
        file.close();
        return true;
    }
    else
    {
        return false;
    }
}
