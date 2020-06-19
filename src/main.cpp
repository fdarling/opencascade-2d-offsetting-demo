#include <cstdio>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pln.hxx>

#include <GCE2d_MakeSegment.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>

#include <Geom2d_OffsetCurve.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>

#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_TShape.hxx>
#include <TopExp_Explorer.hxx>

#include <GC_MakeSegment.hxx>
#include <TopLoc_Location.hxx>

#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>


#include <TopoDS_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopLoc_Datum3D.hxx>

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>

#include <vector>
#include <string>
#include <fstream>

#include "arc_info.h"
#include "collect_segments.h"
#include "export_shape.h"

typedef std::vector<std::string> string_vector;

bool read_input_file(const char * path, std::vector<string_vector>& res)
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

int handle_offset(int argc, const char ** argv)
{
    double offset_value = 0;

    if (sscanf(argv[3], "%lf", &offset_value) != 1 || offset_value == 0)
    {
        printf("FATAL ERROR: invalid offset value %s\n", argv[3]);
        exit(-1);
    }

    GeomAbs_JoinType offset_type = GeomAbs_Arc;

    if (!strcmp("arc", argv[4]))
    {
        offset_type = GeomAbs_Arc;
    }
    else
    if (!strcmp("tangent", argv[4]))
    {
        offset_type = GeomAbs_Tangent;
    }
    else
    if (!strcmp("intersection", argv[4]))
    {
        offset_type = GeomAbs_Intersection;
    }
    else
    {
        printf("FATAL ERROR: invalid offset type. %s\noffset_type must be one of [arc, tangent, intersection]\n", argv[4]);
        exit(-1);
    }

    std::vector<string_vector> segments_arcs;
    if (!read_input_file(argv[1], segments_arcs))
    {
        printf("FATAL ERROR: can not open input file %s\n", argv[1]);
        return false;
    }
    int num_borders = get_num_borders(segments_arcs);

    if (num_borders == -1)
    {
        printf("FATAL ERROR: invalid data in input file %s\n", argv[1]);
        return false;
    }

    std::vector<BRepBuilderAPI_MakeWire> borders(num_borders);

    if (!collect_segments_arcs_to_wires(borders, segments_arcs))
    {
        printf("FATAL ERROR: collect_segments_arcs_to_wires invalid data in input file %s\n", argv[1]);
        return false;
    }

    BRepOffsetAPI_MakeOffset off;
    off.AddWire(borders[0]);
    off.Init(offset_type);
    off.Perform(offset_value);

    const TopoDS_Shape& r = off.Shape();

    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    TopoDS_Shape res = su.Shape();

    FILE * clear_file = fopen(argv[2], "w");
    if (!clear_file)
    {
        printf("FATAL ERROR: can not open output file %s\n", argv[2]);
        return -1;
    }

    FILE * file = fopen(argv[2], "w");

    append_wires_to_file(res, file);
    if (file)
    {
        fclose(file);
    }

    printf("Output file %s is saved.\n", argv[2]);
    return 0;
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


int handle_booleans(int argc, const char ** argv)
{
    TopoDS_Face face_1, face_2;
    if (!load_face_from(argv[1], face_1))
    {
        exit(-1);
    }

    if (!load_face_from(argv[2], face_2))
    {
        exit(-1);
    }

    BRepAlgoAPI_BooleanOperation * booleans = NULL;

    if (!strcmp(argv[4], "intersection"))
    {
        booleans = new BRepAlgoAPI_Common(face_1, face_2);
    }
    else
    if (!strcmp(argv[4], "difference"))
    {
        booleans = new BRepAlgoAPI_Cut(face_1, face_2);
    }
    else
    if (!strcmp(argv[4], "difference_reversed"))
    {
        booleans = new BRepAlgoAPI_Cut(face_2, face_1);
    }
    else
    if (!strcmp(argv[4], "union"))
    {
        booleans = new BRepAlgoAPI_Fuse(face_1, face_2);
    }
    else
    {
        printf("FATAL ERROR: invalid boolean operation type %s. Must be one of [union, intersection, difference, difference_reversed]\n", argv[4]);
        exit(-1);
    }

    booleans->SetRunParallel(true);
    booleans->SetFuzzyValue(1.e-5);
    booleans->SetNonDestructive(true);
    booleans->SetGlue(BOPAlgo_GlueShift);
    booleans->SetCheckInverted(true);

    booleans->Build();

    if (booleans->HasErrors())
    {
        printf("booleans.HasErrors()\n");

        booleans->GetReport()->Dump(std::cout);
        exit(-1);
    }

    if (booleans->HasWarnings())
    {
        printf("booleans.HasWarnings()\n");
        booleans->GetReport()->Dump(std::cout);

    }

    const TopoDS_Shape& r = booleans->Shape();


    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    TopoDS_Shape res = su.Shape();

    FILE * clear_file = fopen(argv[3], "w");
    if (!clear_file)
    {
        printf("FATAL ERROR: can not open output file %s\n", argv[3]);
        exit(-1);
    }
    fclose(clear_file);

    FILE * file = fopen(argv[3], "w");
    int f_ind = 0;
    for(TopExp_Explorer f(res, TopAbs_FACE); f.More(); f.Next())
    {
        TopoDS_Face face = TopoDS::Face(f.Current());
        printf("Face Detected %d\n", f_ind);
        ++f_ind;

        ShapeUpgrade_UnifySameDomain su(face);
        su.Build();
        TopoDS_Shape r = su.Shape();

        append_wires_to_file(r, file);
    }
    if (!f_ind)
    {
        puts("Result is empty set.");
    }
    printf("Output file %s is saved.\n", argv[3]);

    if (booleans)
    {
        delete booleans;
    }

    if (file)
    {
        fclose(file);
    }

    return 0;
}

int main(int argc, const char ** argv)
{

    if (argc != 6)
    {
        printf("Usage A: opencascade_demo offset input_file output_file offset_value offset_type\noffset_type must be one of [arc, tangent, intersection]\n");
        printf("Usage B: opencascade_demo booleans input_file_1 input_file_2 output_file boolean_type\nboolean_type must be one of [union, intersection, difference, difference_reversed]\n");
        exit(-1);
    }

    if (!strcmp(argv[1], "offset"))
    {
        return handle_offset(argc - 1, argv + 1);
    }
    else
    if (!strcmp(argv[1], "booleans"))
    {
        return handle_booleans(argc - 1, argv + 1);
    }
    else
    {
        printf("FATAL ERROR: mode %s in not supported. Supported modes are: [offset, booleans]", argv[1]);
        return -1;
    }

}