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

int get_num_borders(const std::vector<string_vector> & segments_arcs)
{
    int num_borders = 0;
    int num_holes = 0;
    for (size_t i = 0; i < segments_arcs.size(); ++i)
    {
        if (!segments_arcs[i].size())
        {
            continue;
        }
        if (segments_arcs[i][0] == "border")
        {
            ++num_borders;
            if (num_borders > 1)
            {
                printf("FATAL ERROR: multiple borders is not supported\n");
                return -1;
            }
        }
        else
        if (segments_arcs[i][0] == "hole")
        {
            ++num_holes;
        }
    }
    if (!num_borders)
    {
        printf("FATAL ERROR: border is missing\n");
        return -1;
    }
    return num_borders + num_holes;
}

bool collect_segments_arcs_to_wires(std::vector<BRepBuilderAPI_MakeWire> & borders, const std::vector<string_vector> & lines)
{
    int b_ind = 0;
    for (std::vector<string_vector>::size_type line = 0; line < lines.size(); ++line)
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
            borders[b_ind].Add(edge);
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
                if (abs(arc_data[3] - arc_data[4]) == 360)
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
                if (abs(arc_data[3] - arc_data[4]) == 2 * M_PI)
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

            borders[b_ind].Add(edge);
        }
        else
        if (words[0] == "hole")
        {
            ++b_ind;
        }
    }

    for (size_t i = 0; i < borders.size(); ++i)
    {
        bool res = borders[i].IsDone();
        if (!res)
        {
            printf("borders[i].IsDone() failed for i=%zu\n", i);
            return false;
        }
    }
    return true;
}

bool load_face_from(const char * path, TopoDS_Shape & res)
{
    std::vector<string_vector> segments_arcs;
    if (!read_input_file(path, segments_arcs))
    {
        printf("FATAL ERROR: can not open input file %s\n", path);
        return false;
    }

    if (!segments_arcs.size())
    {
        printf("FATAL ERROR: file %s is empty\n", path);
        return false;
    }

    int num_borders = get_num_borders(segments_arcs);

    if (num_borders == -1)
    {
        printf("FATAL ERROR: get_num_borders invalid data in input file %s\n", path);
        return false;
    }

    std::vector<BRepBuilderAPI_MakeWire> borders(num_borders);

    if (!collect_segments_arcs_to_wires(borders, segments_arcs))
    {
        printf("FATAL ERROR: collect_segments_arcs_to_wires invalid data in input file %s\n", path);
        return false;
    }

    BRepBuilderAPI_MakeFace builder(gp_Pln(), borders[0], true);

    for (size_t i = 1; i < borders.size(); ++i)
    {
        builder.Add(borders[i]);
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
