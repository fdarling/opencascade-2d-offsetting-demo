#include "export_shape.h"
#include "arc_info.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Line.hxx>
// #include <Geom_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRep_Tool.hxx>
#include <gp_Circ.hxx>

void append_wires_to_file(const TopoDS_Shape & res, FILE * output_file)
{
    int w_ind = 0;
    for(TopExp_Explorer w(res, TopAbs_WIRE); w.More(); w.Next())
    {
#ifdef DEV_LOG
        printf("\nWire: %d\n", w_ind);
#endif
        if (!w_ind)
        {
            fprintf(output_file, "border\n");
        }
        else
        {
            fprintf(output_file, "hole\n");
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
                fprintf(output_file, "segment %.17g %.17g %.17g %.17g\n", p0.X(), p0.Y(), p1.X(), p1.Y());
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
                fprintf(output_file, "arc %.17g %.17g %.17g %.17g %.17g\n", center.X(), center.Y(), circle->Radius(), angle_start, angle_end);

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
                    fprintf(output_file, "segment %.17g %.17g %.17g %.17g\n", p0.X(), p0.Y(), p1.X(), p1.Y());
                }
                else
                if (trimmed->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_Circle))
                {
                    Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(trimmed->BasisCurve());
                    gp_Pnt center = circle->Circ().Location();
                    double angle_start, angle_end;
                    get_arc_info(center, p0, m, p1, angle_start, angle_end);
#ifdef DEV_LOG
                    printf("trimmed circle detected! R=%f P.x=%f P.y=%f F.x=%f F.y=%f m.x=%f m.y=%f L.x=%f L.y=%f\n", circle->Radius(), center.X(), center.Y(), p0.X(), p0.Y(), m.X(), m.Y(), p1.X(), p1.Y());
                    printf("trimmed angle_start=%lf angle_end=%lf\n", angle_start, angle_end);
#endif
                    fprintf(output_file, "arc %.17g %.17g %.17g %.17g %.17g\n", center.X(), center.Y(), circle->Radius(), angle_start, angle_end);
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
