#include <cstdio>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>

#include <GCE2d_MakeSegment.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>

#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_OffsetCurve.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Iterator.hxx>

#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_TShape.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>

#include <GC_MakeSegment.hxx>
#include <TopLoc_Location.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>


#include <TopoDS_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopLoc_Datum3D.hxx>

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>

using namespace std;

#include <vector>
#include <string>
#include <fstream>

typedef std::vector<std::string> vector_strings;

typedef unsigned char uchar8;
typedef unsigned int  uchar32;
typedef unsigned long uchar64;

//#define DEV_LOG

void print_shape_type(const TopoDS_Shape& sh)
{
	if (sh.ShapeType() == TopAbs_FACE)
	{
		printf("Shape type is TopAbs_FACE\n");
	}
	else
	if (sh.ShapeType() == TopAbs_COMPSOLID)
	{
		printf("Shape type is TopAbs_COMPSOLID\n");
	}
	else
	if (sh.ShapeType() == TopAbs_COMPOUND)
	{
		printf("Shape type is TopAbs_COMPOUND\n");
	}
	else
	if (sh.ShapeType() == TopAbs_WIRE)
	{
		printf("Shape type is TopAbs_WIRE\n");
	}
	else
	if (sh.ShapeType() == TopAbs_EDGE)
	{
		printf("Shape type is TopAbs_EDGE\n");
	}
	else
	if (sh.ShapeType() == TopAbs_VERTEX)
	{
		printf("Shape type is TopAbs_VERTEX\n");
	}
}

bool read_input_file(const char * path, std::vector<vector_strings>& res)
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
	        res.push_back(vector_strings());			
			size_t prev = 0, pos = 0;
		    do
		    {
		        pos = line.find(" ", prev);
		        if (pos == string::npos)
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

bool collect_segments_arcs_to_wires(std::vector<BRepBuilderAPI_MakeWire> & borders, const std::vector<vector_strings>& segments_arcs)
{	
	int b_ind = 0;
	for (size_t i = 0; i < segments_arcs.size(); ++i)
	{
		if (segments_arcs[i][0] == "segment")
		{
			if (segments_arcs[i].size() != 5)
			{
				printf("line %zu with segment data must contain 5 entries, but only %lu found.\n", i, segments_arcs[i].size());
				return false;
			}
			
			double pts[4];
			for (size_t j = 1; j < 5; ++j)
			{
				if (sscanf(segments_arcs[i][j].c_str(), "%lf", &pts[j - 1]) != 1)
				{
					printf("line %zu with segment data invalid double entry in pos %zu\n", i, j);
					return false;
				}
			}			
			TopoDS_Vertex vtxs[2] = {BRepBuilderAPI_MakeVertex(gp_Pnt(pts[0], pts[1], 0)), BRepBuilderAPI_MakeVertex(gp_Pnt(pts[2], pts[3], 0))};
			TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(vtxs[0], vtxs[1]);
			borders[b_ind].Add(edge);			
		}
		else
		if (segments_arcs[i][0] == "arc" || segments_arcs[i][0] == "arc_degrees")
		{
			if (segments_arcs[i].size() != 6)
			{
				printf("line %zu with arc data must contain 6 entries, but only %lu found.\n", i, segments_arcs[i].size());
				return false;
			}			
			// c_x c_y R angle_start angle_end
			double arc_data[5];
			for (size_t j = 1; j < 6; ++j)
			{
				if (sscanf(segments_arcs[i][j].c_str(), "%lf", &arc_data[j - 1]) != 1)
				{
					printf("line %zu with arc data invalid double entry in pos %zu\n", i, j);
					return false;
				}
			}	
			bool full_circle = false;
			
			if (segments_arcs[i][0] == "arc_degrees")
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
		if (segments_arcs[i][0] == "hole")
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


void get_arc_info(const gp_Pnt & center, const gp_Pnt & f, const gp_Pnt & m, const gp_Pnt & l, double& angle_start, double& angle_end)
{
	angle_start = atan2(f.Y() - center.Y(), f.X() - center.X());
	double angle_mid   = atan2(m.Y() - center.Y(), m.X() - center.X());
	angle_end   = atan2(l.Y() - center.Y(), l.X() - center.X());
#ifdef DEV_LOG	
	printf("angle_start: %lf angle_mid: %lf angle_end: %lf diff: %lf\n", angle_start, angle_mid, angle_end, abs(angle_start - angle_end));
#endif	
	double v = abs(angle_start - angle_end);
	double t = 0.00001;
	if ((v > t) && (v < 2 * M_PI - t))
	{
		//puts("CASE A");
		if (angle_start < 0)
		{
			angle_start += 2 * M_PI;
		}
		if (angle_end < 0)
		{
			angle_end += 2 * M_PI;
		}
		if (angle_start > angle_end)
		{
			std::swap(angle_start, angle_end);
		}
		if (angle_mid < 0)
		{
			angle_mid += 2 * M_PI;
		}
	
		if (angle_mid < angle_start || angle_mid > angle_end)
		{
			std::swap(angle_start, angle_end);
		}		
	}
	else
	{
		//puts("CASE B");
		angle_start = 0;
		angle_end = 2 * M_PI;
	}
}
	
void append_wires_to_file(const TopoDS_Shape& res, FILE * ouput_file)
{	
	int w_ind = 0;
    for(TopExp_Explorer w(res, TopAbs_WIRE); w.More(); w.Next())
	{
#ifdef DEV_LOG			
		printf("\nWire: %d\n", w_ind);
#endif
		if (!w_ind)
		{
			fprintf(ouput_file, "border\n");
		}
		else
		{
			fprintf(ouput_file, "hole\n");
		}

		++w_ind;				
		int e_ind = 0;
		TopLoc_Location L;
		Standard_Real First, Last;
		
		TopoDS_Wire wire = TopoDS::Wire(w.Current());
		
	    for(TopExp_Explorer e(w.Value(), TopAbs_EDGE); e.More(); e.Next())
		{

			TopoDS_Edge edge = TopoDS::Edge(e.Current());
			Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, L, First, Last);
			
			
			gp_Pnt p0, p1, m;
			curve->D0(First, p0);
			curve->D0( (First + Last) * 0.5, m);
			curve->D0(Last, p1);
			
			gp_XYZ axis(0, 0, 1.0);
			Standard_Real angle;

			//L.DumpJson(std::cout);
			if (!L.Transformation().GetRotation(axis, angle))
			{
				angle = 0;
			}
			
			const gp_XYZ& tr = L.Transformation().TranslationPart();
			
#ifdef DEV_LOG					
			printf("\nEdge: %d p0.x=%.17g p0.y=%.17g p1.x=%.17g p1.y=%.17g\n", e_ind, p0.X(), p0.Y(), p1.X(), p1.Y());
#endif			
			++e_ind;
			
	        if(curve->DynamicType() == STANDARD_TYPE(Geom_Line))
			{
#ifdef DEV_LOG					
				printf("segment detected! %f %f %f %f\n", p0.X(), p0.Y(), p1.X(), p1.Y());
#endif				
				fprintf(ouput_file, "segment %.17g %.17g %.17g %.17g\n", p0.X(), p0.Y(), p1.X(), p1.Y());
	        }
			else
			if(curve->DynamicType() == STANDARD_TYPE(Geom_Circle))
			{
				Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
				gp_Pnt center = circle->Circ().Location();				
				double angle_start, angle_end;
				get_arc_info(center, p0, m, p1, angle_start, angle_end);
#ifdef DEV_LOG	
				printf("circle detected! R=%f P.x=%f P.y=%f F.x=%f F.y=%f m.x=%f m.y=%f L.x=%f L.y=%f\n", circle->Radius(), center.X(), center.Y(), p0.X(), p0.Y(), m.X(), m.Y(), p1.X(), p1.Y());
				printf("angle_start=%lf angle_end=%lf\n", angle_start, angle_end);
#endif				
				fprintf(ouput_file, "arc %.17g %.17g %.17g %.17g %.17g\n", center.X(), center.Y(), circle->Radius(), angle_start, angle_end);
								
			}
			else
			if (curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
			{
				Handle(Geom_TrimmedCurve) trimmed = Handle(Geom_TrimmedCurve)::DownCast(curve);
				
				if (trimmed->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_Line))
				{
#ifdef DEV_LOG						
					printf("segment detected! %f %f %f %f\n", p0.X(), p0.Y(), p1.X(), p1.Y());
#endif					
					fprintf(ouput_file, "segment %.17g %.17g %.17g %.17g\n", p0.X(), p0.Y(), p1.X(), p1.Y());
				}
				else
				if (trimmed->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_Circle))
				{
					Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(trimmed->BasisCurve());
					gp_Pnt center = circle->Circ().Location();
					double angle_start, angle_end;
					get_arc_info(center, p0, m, p1, angle_start, angle_end);
#ifdef DEV_LOG						
					printf("trmmed circle detected! R=%f P.x=%f P.y=%f F.x=%f F.y=%f m.x=%f m.y=%f L.x=%f L.y=%f\n", circle->Radius(), center.X(), center.Y(), p0.X(), p0.Y(), m.X(), m.Y(), p1.X(), p1.Y());
					printf("trimmed angle_start=%lf angle_end=%lf\n", angle_start, angle_end);
#endif					
					fprintf(ouput_file, "arc %.17g %.17g %.17g %.17g %.17g\n", center.X(), center.Y(), circle->Radius(), angle_start, angle_end);						
				}
				else
				{
					printf("FATAL ERROR A: not supported type %s!\n", trimmed->BasisCurve()->DynamicType()->Name());
					exit(-1);					
				}				
			}
			else			
			{
				printf("FATAL ERROR B: not supported type %s!\n", curve->DynamicType()->Name());
				exit(-1);
			}			
			
			/*
			int v_ind = 0;
		    for(TopExp_Explorer v(e.Value(), TopAbs_VERTEX); v.More(); v.Next())
			{				
				TopoDS_Vertex vtx = TopoDS::Vertex(v.Current());
				gp_Pnt p = BRep_Tool::Pnt(vtx);
				
				printf("Vertex: %d %f %f\n", v_ind, p.X(), p.Y());
				++v_ind;
			}
			*/
		}		
	}

}

int get_num_borders(const std::vector<vector_strings>& segments_arcs)
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
				printf("FATAL ERROR: mutiple borders is not supported\n");
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
	
	std::vector<vector_strings> segments_arcs;
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
	std::vector<vector_strings> segments_arcs;
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