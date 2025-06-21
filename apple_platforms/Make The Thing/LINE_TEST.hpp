//
//  LINE_TEST.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 5/10/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef LINE_TEST_h
#define LINE_TEST_h

{
    sd::rewind_layer(renderer, LAYER_LABEL_LINE_TEST);
    sd::set_render_layer(renderer, LAYER_LABEL_LINE_TEST);
    
//            {
//                sd::begin_polygon(renderer);
//                vec3 off = vec3(300.0f, 400.0f, 900.0f);
//                sd::vertex_v3(renderer, off);
//                sd::vertex_v3(renderer, off + vec3(100,-100,0));
//                sd::vertex_v3(renderer, off + vec3(200,0,0));
//                sd::vertex_v3(renderer, off + vec3(300,-100,0));
//                sd::vertex_v3(renderer, off + vec3(400,0,0));
//                usize N = 5;
//                auto* tri_strip = sd::end_polygon(renderer);
//
//                constexpr bool manual = false;
//                if constexpr (manual) {
//                    auto indices = sd::index_list(renderer, tri_strip, N * 3);
//                    sd::set_triangle_indices(renderer, indices, 0, 0,1,2);
//                    sd::set_triangle_indices(renderer, indices, 1, 1,2,3);
//                    sd::set_triangle_indices(renderer, indices, 2, 2,3,4);
//                } else {
//                    sd::triangle_strip_to_indexed_triangles_in_place_in_place(renderer, tri_strip);
//                }
//            }
    {
        auto vg = nvgGetGlobalContext();
        mat4 M = Mat4(1.0f);
        M = m::translate(M, vec3(300.0f, 0.0f, 0.0f));
        nvgSetModelTransform(vg, value_ptr(M));

        nvgSave(vg);
        //nvgSetViewTransform(vg, value_ptr(dt_world->cam.view_transform));
        nvgBeginPath(vg);
        float32 SC = 1;
        float32 swidth = m::sin01(core->time_seconds / 100) * SC;

        nvgStrokeColor(vg, nvgRGBf(1, 0, 0));
        nvgLineJoin(vg, NVG_ROUND);
        nvgLineCap(vg, NVG_ROUND);
        nvgStrokeWidth(vg, swidth);
        nvgMoveTo(vg, 300, 400);
//        nvgLineTo(vg, 500, 400);
//        nvgLineTo(vg, 700, 700);
        float w_inc = .27 * SC;
        float64 bound = 10;

        //auto RNDOM = MTT_Random_Float64_value;
        auto RNDOM = []() { return 1.0; };

        auto RNDOM_OFF = [&]() { return (MTT_Random_Float64_value() * 10 * m::sin01(core->time_seconds / 100)) + 1; };
        //auto RNDOM_OFF = []() { return 1.0; };

        if ((false)) {

            swidth += w_inc;
            vec2 prev = vec2(300, 400);
            vec2 tgt = vec2(500, 400);
            for (float64 i = 0; i <= bound; i += 1) {
                vec2 pos = m::lerp(prev, tgt, i / bound);

                pos.x += (RNDOM() -.5) * 50.0 * swidth;
                pos.y += (RNDOM() -.5) * 50.0 * swidth;

                nvgLineToEX(vg, pos.x, pos.y, 999.0f, swidth + RNDOM_OFF(), nvgRGBf(1, 0, 0));
            }

            swidth += w_inc;
            prev = tgt;
            tgt = vec2(700, 700);
            for (float64 i = 0; i <= bound; i += 1) {
                vec2 pos = m::lerp(prev, tgt, i / bound);

                pos.x += (RNDOM() -.5) * 50.0 * swidth;
                pos.y += (RNDOM() -.5) * 50.0 * swidth;

                nvgLineToEX(vg, pos.x, pos.y, 999.0f, swidth + RNDOM_OFF(), nvgRGBf(1, 0, 0));
            }


            swidth += w_inc * 2;
            prev = tgt;
            tgt = vec2(600, 750.25);
            for (float64 i = 0; i <= bound; i += 1) {
                vec2 pos = m::lerp(prev, tgt, i / bound);

                pos.x += (RNDOM() -.5) * 50.0 * swidth;
                pos.y += (RNDOM() -.5) * 50.0 * swidth;

                nvgLineToEX(vg, pos.x, pos.y, 999.0f, swidth + RNDOM_OFF(), nvgRGBf(1, 0, 0));
            }



            swidth -= w_inc;
            prev = tgt;
            tgt = vec2(500, 700);
            for (float64 i = 0; i <= bound; i += 1) {
                vec2 pos = m::lerp(prev, tgt, i / bound);

                pos.x += (RNDOM() -.5) * 50.0 * swidth;
                pos.y += (RNDOM() -.5) * 50.0 * swidth;

                nvgLineToEX(vg, pos.x, pos.y, 999.0f, swidth + RNDOM_OFF(), nvgRGBf(1, 0, 0));
            }

            nvgStrokeEX(vg);

        } else {
            sd::Line_Renderer r;
            r.beginLine();


            float32 SC = 1;
            float32 swidth = m::sin01(core->time_seconds / 100) * SC;


            r.r = 1;
            r.lineVertex(300, 400);
            r.depth = 999.9f;

            //        nvgLineTo(vg, 500, 400);
            //        nvgLineTo(vg, 700, 700);
            float w_inc = .27 * SC;
            float64 bound = 10;

            //auto RNDOM = MTT_Random_Float64_value;
            auto RNDOM = []() { return 1.0; };

            auto RNDOM_OFF = [&]() { return (MTT_Random_Float64_value() * 10 * m::sin01(core->time_seconds / 100)) + 1; };
            //auto RNDOM_OFF = []() { return 1.0; };

            if ((true)) {

                swidth += w_inc;
                vec2 prev = vec2(300, 400);
                vec2 tgt = vec2(500, 400);
                for (float64 i = 0; i <= bound; i += 1) {
                    vec2 pos = m::lerp(prev, tgt, i / bound);

                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
                    pos.y += (RNDOM() -.5) * 50.0 * swidth;

                    r.depth = 990.0f;
                    r.r = swidth + RNDOM_OFF();
                    r.lineVertex(pos.x, pos.y);
                }

                swidth += w_inc;
                prev = tgt;
                tgt = vec2(700, 700);
                for (float64 i = 0; i <= bound; i += 1) {
                    vec2 pos = m::lerp(prev, tgt, i / bound);

                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
                    pos.y += (RNDOM() -.5) * 50.0 * swidth;

                    r.depth = 990.0f;
                    r.r = swidth + RNDOM_OFF();
                    r.lineVertex(pos.x, pos.y);
                }


                swidth += w_inc * 2;
                prev = tgt;
                tgt = vec2(600, 750.25);
                for (float64 i = 0; i <= bound; i += 1) {
                    vec2 pos = m::lerp(prev, tgt, i / bound);

                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
                    pos.y += (RNDOM() -.5) * 50.0 * swidth;

                    r.depth = 990.0f;
                    r.r = swidth + RNDOM_OFF();
                    r.lineVertex(pos.x, pos.y);
                }

                swidth -= w_inc;
                prev = tgt;
                tgt = vec2(500, 700);
                for (float64 i = 0; i <= bound; i += 1) {
                    vec2 pos = m::lerp(prev, tgt, i / bound);

                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
                    pos.y += (RNDOM() -.5) * 50.0 * swidth;

                    r.depth = 990.0f;
                    r.r = swidth + RNDOM_OFF();
                    r.lineVertex(pos.x, pos.y);
                }


                nvgBeginPath(vg);
                nvgStrokeEXWithPointList(vg, r.points.data(), r.points.size());
                /*
                 NVGpoint* pt = nvg__addPoint(ctx, p[0], p[1], NVG_PT_CORNER);
                 if (pt != NULL) {
                     pt->width = p[3];
                     pt->r =p[4];
                     pt->g = p[5];
                     pt->b = p[6];
                     pt->a = p[7];
                 }
                 */
            }
        }

        struct IN {
            MTT_Core* core;
            sd::Renderer* renderer;
        } in;
        in.core = core;
        in.renderer = renderer;

        sd::begin_polygon(renderer);
        NVG_foreach_point(vg, 0, [](void* state, NVGvertex* vertex) {
            auto* in = (IN*)state;
            MTT_Core* core = in->core;
            sd::Renderer* renderer = in->renderer;

            sd::vec2 p = {vertex->x - 200, vertex->y};

            sd::vertex_v2(renderer, p);


        }, &in);

        auto tri_strip = sd::end_polygon(renderer);
        sd::triangle_strip_to_indexed_triangles_in_place(renderer, tri_strip);

        nvgRestore(vg);
    }
    
//            if ((false)) {
//                sd::begin_path(renderer);
//
//                sd::save(renderer);
//
//
//                float32 SC = 1;
//                float32 swidth = m::sin01(core->time_seconds / 100) * SC;
//
//                sd::set_path_radius(renderer, swidth);
//
//                sd::path_vertex_v2(renderer, vec2(300, 400));
//
//                //        nvgLineTo(vg, 500, 400);
//                //        nvgLineTo(vg, 700, 700);
//                float w_inc = .27 * SC;
//                float64 bound = 10;
//
//                //auto RNDOM = MTT_Random_Float64_value;
//                auto RNDOM = []() { return 1.0; };
//
//                auto RNDOM_OFF = [&]() { return (MTT_Random_Float64_value() * 10 * m::sin01(core->time_seconds / 100)) + 1; };
//                //auto RNDOM_OFF = []() { return 1.0; };
//
//                swidth += w_inc;
//                vec2 prev = vec2(300, 400);
//                vec2 tgt = vec2(500, 400);
//                for (float64 i = 0; i <= bound; i += 1) {
//                    vec2 pos = m::lerp(prev, tgt, i / bound);
//
//                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
//                    pos.y += (RNDOM() -.5) * 50.0 * swidth;
//
//                    sd::path_vertex_v3(renderer, vec3(pos.x, pos.y, 999.0f));
//                    sd::set_path_radius(renderer, swidth + RNDOM_OFF());
//                }
//
//                swidth += w_inc;
//                prev = tgt;
//                tgt = vec2(700, 700);
//                for (float64 i = 0; i <= bound; i += 1) {
//                    vec2 pos = m::lerp(prev, tgt, i / bound);
//
//                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
//                    pos.y += (RNDOM() -.5) * 50.0 * swidth;
//
//                    sd::path_vertex_v3(renderer, vec3(pos.x, pos.y, 999.0f));                        sd::set_path_radius(renderer, swidth + RNDOM_OFF());
//                }
//
//
//                swidth += w_inc * 2;
//                prev = tgt;
//                tgt = vec2(600, 750.25);
//                for (float64 i = 0; i <= bound; i += 1) {
//                    vec2 pos = m::lerp(prev, tgt, i / bound);
//
//                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
//                    pos.y += (RNDOM() -.5) * 50.0 * swidth;
//
//                    sd::path_vertex_v3(renderer, vec3(pos.x, pos.y, 999.0f));                        sd::set_path_radius(renderer, swidth + RNDOM_OFF());
//                }
//
//
//
//                swidth -= w_inc;
//                prev = tgt;
//                tgt = vec2(500, 700);
//                for (float64 i = 0; i <= bound; i += 1) {
//                    vec2 pos = m::lerp(prev, tgt, i / bound);
//
//                    pos.x += (RNDOM() -.5) * 50.0 * swidth;
//                    pos.y += (RNDOM() -.5) * 50.0 * swidth;
//
//                    sd::path_vertex_v3(renderer, vec3(pos.x, pos.y, 999.0f));                        sd::set_path_radius(renderer, swidth + RNDOM_OFF());
//                }
//
//                sd::end_path(renderer);
//                sd::restore(renderer);
//
//            }

}


#endif /* LINE_TEST_h */
