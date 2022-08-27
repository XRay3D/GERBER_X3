/*

ABOUT:

    A fast single file 2D voronoi diagram generator.

HISTORY:

    0.6     2018-10-21  - Removed JCV_CEIL/JCV_FLOOR/JCV_FABS
                        - Optimizations: Fewer indirections, better beach head approximation
    0.5     2018-10-14  - Fixed issue where the graph edge had the wrong edge assigned (issue #28)
                        - Fixed issue where a point was falsely passing the jcv_is_valid() test (issue #22)
                        - Fixed jcv_diagraget_edges_() so it now returns _all_ edges (issue #28)
                        - Added jcv_diagraget_next_edge_() to skip zero length edges (issue #10)
                        - Added defines JCV_CEIL/JCV_FLOOR/JCV_FLT_MAX for easier configuration
    0.4     2017-06-03  - Increased the max number of events that are preallocated
    0.3     2017-04-16  - Added clipping box as input argument (Automatically calcuated if needed)
                        - Input points are pruned based on bounding box
    0.2     2016-12-30  - Fixed issue of edges not being closed properly
                        - Fixed issue when having many events
                        - Fixed edge sorting
                        - Code cleanup
    0.1                 Initial version

LICENSE:

    The MIT License (MIT)

    Copyright (c) 2018 Mathias Westerdahl

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

DISCLAIMER:

    This software is supplied "AS IS" without any warranties and support

USAGE:

    The input points are pruned if

        * There are duplicates points
        * The input points are outside of the bounding box

    The input bounding box is optional (calculated automatically)

    The input domain is (-FLT_MAX, FLT_MAX] (for floats)

    The api consists of these functions:

    void jcv_diagragenerate_( int nupoints_, const jcv_point* points, const jcv_rect* rect, jcv_diagram* diagram );
    void jcv_diagragenerate_useralloc_( int nupoints_, const jcv_point* points, const jcv_rect* rect, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram );
    void jcv_diagrafree_( jcv_diagram* diagram );

    const jcv_site* jcv_diagraget_sites_( const jcv_diagram* diagram );
    const jcv_edge* jcv_diagraget_edges_( const jcv_diagram* diagram );
    const jcv_edge* jcv_diagraget_next_edge_( const jcv_edge* edge );

    An example usage:

    #define JC_VORONOI_IMPLEMENTATION
    // If you wish to use doubles
    //#define JCV_REAL_TYPE double
    //#define JCV_ATAN2 atan2
    //#define JCV_FLT_MAX 1.7976931348623157E+308
    #include "jc_voronoi.h"

    void draw_edges(const jcv_diagram* diagram);
    void draw_cells(const jcv_diagram* diagram);

    void generate_and_draw(int numpoints, const jcv_point* points)
    {
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagragenerate_(count, points, 0, &diagram);

        draw_edges(diagram);
        draw_cells(diagram);

        jcv_diagrafree_( &diagram );
    }

    void draw_edges(const jcv_diagram* diagram)
    {
        // If all you need are the edges
        const jcv_edge* edge = jcv_diagraget_edges_( diagram );
        while( edge )
        {
            draw_line(edge->pos[0], edge->pos[1]);
            edge = jcv_diagraget_next_edge_(edge);
        }
    }

    void draw_cells(const jcv_diagram* diagram)
    {
        // If you want to draw triangles, or relax the diagram,
        // you can iterate over the sites and get all edges easily
        const jcv_site* sites = jcv_diagraget_sites_( diagram );
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];

            const jcv_graphedge* e = site->edges;
            while( e )
            {
                draw_triangle( site->p, e->pos[0], e->pos[1]);
                e = e->next;
            }
        }
    }

    // Here is a simple example of how to do the relaxations of the cells
    void relax_points(const jcv_diagram* diagram, jcv_point* points)
    {
        const jcv_site* sites = jcv_diagraget_sites_(diagram);
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];
            jcv_point sum = site->p;
            int count = 1;

            const jcv_graphedge* edge = site->edges;

            while( edge )
            {
                sum.x += edge->pos[0].x;
                sum.y += edge->pos[0].y;
                ++count;
                edge = edge->next;
            }

            points[site->index].x = sum.x / count;
            points[site->index].y = sum.y / count;
        }
    }

 */
#pragma once

#include "math.h"
#include "stddef.h"
#include "stdlib.h"
#include <limits>

#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JCV_REAL_TYPE
#define JCV_REAL_TYPE double
#endif

#ifndef JCV_ATAN2
#define JCV_ATAN2(_Y_, _X_) atan2(_Y_, _X_)
#endif

#ifndef JCV_SQRT
#define JCV_SQRT(_X_) sqrt(_X_)
#endif

#ifndef JCV_PI
#define JCV_PI 3.14159265358979323846264338327950288f
#endif

#ifndef JCV_FLT_MAX
#define JCV_FLT_MAX std::numeric_limits<JCV_REAL_TYPE>::max() // 3.402823466e+38F
#endif

#ifndef JCV_EDGE_INTERSECT_THRESHOLD
#define JCV_EDGE_INTERSECT_THRESHOLD 1.0e-10F
#endif

typedef JCV_REAL_TYPE jcv_real;

#pragma pack(push, 1)

typedef struct _jcv_point {
    jcv_real x;
    jcv_real y;
    int id = -1;
    int id2 = -1;
} jcv_point;

typedef struct _jcv_graphedge {
    struct _jcv_graphedge* next;
    struct _jcv_edge* edge;
    struct _jcv_site* neighbor;
    jcv_point pos[2];
    jcv_real angle;
} jcv_graphedge;

typedef struct _jcv_site {
    jcv_point p;
    int index;            // Index into the original list of points
    jcv_graphedge* edges; // The half edges owned by the cell
} jcv_site;

typedef struct _jcv_edge {
    struct _jcv_edge* next;
    jcv_site* sites[2];
    jcv_point pos[2];
    jcv_real a;
    jcv_real b;
    jcv_real c;
} jcv_edge;

typedef struct _jcv_rect {
    jcv_point min;
    jcv_point max;
} jcv_rect;

typedef struct _jcv_diagram {
    struct _jcv_context_internal* internal = nullptr;
    jcv_edge* edges;
    jcv_site* sites;
    int numsites;
    jcv_point min;
    jcv_point max;
} jcv_diagram;

#pragma pack(pop)

/**
 * Uses malloc
 * If rect is null, an automatic bounding box is calculated, with an extra padding of 10 units
 * If rect is non null, all points will be culled against the bounding rect, and all edges will be clipped against it.
 */
extern void jcv_diagragenerate_(size_t nupoints_, const jcv_point* points, const jcv_rect* rect, void*, jcv_diagram* diagram);

typedef void* (*FJCVAllocFn)(void* userctx, size_t size);
typedef void (*FJCVFreeFn)(void* userctx, void* p);

// Same as above, but allows the client to use a custom allocator
extern void jcv_diagragenerate_useralloc_(size_t nupoints_, const jcv_point* points, const jcv_rect* rect, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram);

// Uses free (or the registered custom free function)
extern void jcv_diagrafree_(jcv_diagram* diagram);

// Returns an array of sites, where each index is the same as the original input point array.
extern const jcv_site* jcv_diagraget_sites_(const jcv_diagram* diagram);

// Returns a linked list of all the voronoi edges
// excluding the ones that lie on the borders of the bounding box.
// For a full list of edges, you need to iterate over the sites, and their graph edges.
extern const jcv_edge* jcv_diagraget_edges_(const jcv_diagram* diagram);

// Iterates over a list of edges, skipping invalid edges (where p0==p1)
extern const jcv_edge* jcv_diagraget_next_edge_(const jcv_edge* edge);

#ifdef __cplusplus
}
#endif

// Copyright (c) 2015-2019 Mathias Westerdahl
// For LICENSE (MIT), USAGE or HISTORY, see bottom of file

//#pragma once

//#ifdef __cplusplus
// extern "C" {
//#endif

//#ifndef JCV_REAL_TYPE
//#define JCV_REAL_TYPE float
//#endif

//#ifndef JCV_ATAN2
//#define JCV_ATAN2(_Y_, _X_) atan2f(_Y_, _X_)
//#endif

//#ifndef JCV_SQRT
//#define JCV_SQRT(_X_) sqrtf(_X_)
//#endif

//#ifndef JCV_PI
//#define JCV_PI 3.14159265358979323846264338327950288f
//#endif

//#ifndef JCV_FLT_MAX
//#define JCV_FLT_MAX 3.402823466e+38F
//#endif

//#ifndef JCV_EDGE_INTERSECT_THRESHOLD
//// Fix for Issue #40
//#define JCV_EDGE_INTERSECT_THRESHOLD 1.0e-10F
//#endif

// typedef JCV_REAL_TYPE jcv_real;

// typedef struct _jcv_point jcv_point;
// typedef struct _jcv_rect jcv_rect;
// typedef struct _jcv_site jcv_site;
// typedef struct _jcv_edge jcv_edge;
// typedef struct _jcv_graphedge jcv_graphedge;
// typedef struct _jcv_diagram jcv_diagram;
// typedef struct _jcv_clipper jcv_clipper;
// typedef struct _jcv_context_internal jcv_context_internal;

///// Tests if a point is inside the final shape
// typedef int (*jcv_clip_test_point_fn)(const jcv_clipper* clipper, const jcv_point p);
///** Given an edge, and the clipper, calculates the e->pos[0] and e->pos[1]
// * Returns 0 if not successful
// */
// typedef int (*jcv_clip_edge_fn)(const jcv_clipper* clipper, jcv_edge* e);
///** Given the clipper, the site and the last edge,
// * closes any gaps in the polygon by adding new edges that follow the bounding shape
// * The internal context is use when allocating new edges.
// */
// typedef void (*jcv_clip_fillgap_fn)(const jcv_clipper* clipper, jcv_context_internal* allocator, jcv_site* s);

///**
// * Uses malloc
// * If a clipper is not supplied, a default box clipper will be used
// * If rect is null, an automatic bounding box is calculated, with an extra padding of 10 units
// * All points will be culled against the bounding rect, and all edges will be clipped against it.
// */
// extern void jcv_diagragenerate_(int nupoints_, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, jcv_diagram* diagram);

// typedef void* (*FJCVAllocFn)(void* userctx, size_t size);
// typedef void (*FJCVFreeFn)(void* userctx, void* p);

//// Same as above, but allows the client to use a custom allocator
// extern void jcv_diagragenerate_useralloc_(int nupoints_, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram);

//// Uses free (or the registered custom free function)
// extern void jcv_diagrafree_(jcv_diagram* diagram);

//// Returns an array of sites, where each index is the same as the original input point array.
// extern const jcv_site* jcv_diagraget_sites_(const jcv_diagram* diagram);

//// Returns a linked list of all the voronoi edges
//// excluding the ones that lie on the borders of the bounding box.
//// For a full list of edges, you need to iterate over the sites, and their graph edges.
// extern const jcv_edge* jcv_diagraget_edges_(const jcv_diagram* diagram);

//// Iterates over a list of edges, skipping invalid edges (where p0==p1)
// extern const jcv_edge* jcv_diagraget_next_edge_(const jcv_edge* edge);

//// For the default clipper
// extern int jcv_boxshape_test(const jcv_clipper* clipper, const jcv_point p);
// extern int jcv_boxshape_clip(const jcv_clipper* clipper, jcv_edge* e);
// extern void jcv_boxshape_fillgaps(const jcv_clipper* clipper, jcv_context_internal* allocator, jcv_site* s);

//#pragma pack(push, 1)

// struct _jcv_point {
//     jcv_real x;
//     jcv_real y;
//      int id = 0;
// };

// struct _jcv_graphedge {
//     struct _jcv_graphedge* next;
//     struct _jcv_edge* edge;
//     struct _jcv_site* neighbor;
//     jcv_point pos[2];
//     jcv_real angle;
// };

// struct _jcv_site {
//     jcv_point p;
//     int index; // Index into the original list of points
//     jcv_graphedge* edges; // The half edges owned by the cell
// };

//// The coefficients a, b and c are from the general line equation: ax * by + c = 0
// struct _jcv_edge {
//     struct _jcv_edge* next;
//     jcv_site* sites[2];
//     jcv_point pos[2];
//     jcv_real a;
//     jcv_real b;
//     jcv_real c;
// };

// struct _jcv_rect {
//     jcv_point min;
//     jcv_point max;
// };

// struct _jcv_clipper {
//     jcv_clip_test_point_fn test_fn;
//     jcv_clip_edge_fn clip_fn;
//     jcv_clip_fillgap_fn fill_fn;
//     jcv_point min; // The bounding rect min
//     jcv_point max; // The bounding rect max
//     void* ctx; // User defined context
// };

// struct _jcv_diagram {
//     jcv_context_internal* internal;
//     jcv_edge* edges;
//     jcv_site* sites;
//     int numsites;
//     jcv_point min;
//     jcv_point max;
// };

//#pragma pack(pop)

//#ifdef __cplusplus
//}
//#endif

///*
// ABOUT:
//     A fast single file 2D voronoi diagram generator
// HISTORY:
//     0.7     2019-10-25  - Added support for clipping against convex polygons
//                         - Added JCV_EDGE_INTERSECT_THRESHOLD for edge intersections
//                         - Fixed issue where the bounds calculation wasn’t considering all points
//     0.6     2018-10-21  - Removed JCV_CEIL/JCV_FLOOR/JCV_FABS
//                         - Optimizations: Fewer indirections, better beach head approximation
//     0.5     2018-10-14  - Fixed issue where the graph edge had the wrong edge assigned (issue #28)
//                         - Fixed issue where a point was falsely passing the jcv_is_valid() test (issue #22)
//                         - Fixed jcv_diagraget_edges_() so it now returns _all_ edges (issue #28)
//                         - Added jcv_diagraget_next_edge_() to skip zero length edges (issue #10)
//                         - Added defines JCV_CEIL/JCV_FLOOR/JCV_FLT_MAX for easier configuration
//     0.4     2017-06-03  - Increased the max number of events that are preallocated
//     0.3     2017-04-16  - Added clipping box as input argument (Automatically calculated if needed)
//                         - Input points are pruned based on bounding box
//     0.2     2016-12-30  - Fixed issue of edges not being closed properly
//                         - Fixed issue when having many events
//                         - Fixed edge sorting
//                         - Code cleanup
//     0.1                 Initial version
// LICENSE:
//     The MIT License (MIT)
//     Copyright (c) 2015-2019 Mathias Westerdahl
//     Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
//     in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
//     furnished to do so, subject to the following conditions:
//     The above copyright notice and this permission notice shall be included in all
//     copies or substantial portions of the Software.
//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//     SOFTWARE.
// DISCLAIMER:
//     This software is supplied "AS IS" without any warranties and support
// USAGE:
//     The input points are pruned if
//         * There are duplicates points
//         * The input points are outside of the bounding box (i.e. fail the clipping test function)
//         * The input points are rejected by the clipper's test function
//     The input bounding box is optional (calculated automatically)
//     The input domain is (-FLT_MAX, FLT_MAX] (for floats)
//     The api consists of these functions:
//     void jcv_diagragenerate_( int nupoints_, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, jcv_diagram* diagram );
//     void jcv_diagragenerate_useralloc_( int nupoints_, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, const jcv_clipper* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram );
//     void jcv_diagrafree_( jcv_diagram* diagram );
//     const jcv_site* jcv_diagraget_sites_( const jcv_diagram* diagram );
//     const jcv_edge* jcv_diagraget_edges_( const jcv_diagram* diagram );
//     const jcv_edge* jcv_diagraget_next_edge_( const jcv_edge* edge );
//     An example usage:
//     #define JC_VORONOI_IMPLEMENTATION
//     // If you wish to use doubles
//     //#define JCV_REAL_TYPE double
//     //#define JCV_ATAN2 atan2
//     //#define JCV_FLT_MAX 1.7976931348623157E+308
//     #include "jc_voronoi.h"
//     void draw_edges(const jcv_diagram* diagram);
//     void draw_cells(const jcv_diagram* diagram);
//     void generate_and_draw(int numpoints, const jcv_point* points)
//     {
//         jcv_diagram diagram;
//         memset(&diagram, 0, sizeof(jcv_diagram));
//         jcv_diagragenerate_(count, points, 0, 0, &diagram);
//         draw_edges(diagram);
//         draw_cells(diagram);
//         jcv_diagrafree_( &diagram );
//     }
//     void draw_edges(const jcv_diagram* diagram)
//     {
//         // If all you need are the edges
//         const jcv_edge* edge = jcv_diagraget_edges_( diagram );
//         while( edge )
//         {
//             draw_line(edge->pos[0], edge->pos[1]);
//             edge = jcv_diagraget_next_edge_(edge);
//         }
//     }
//     void draw_cells(const jcv_diagram* diagram)
//     {
//         // If you want to draw triangles, or relax the diagram,
//         // you can iterate over the sites and get all edges easily
//         const jcv_site* sites = jcv_diagraget_sites_( diagram );
//         for( int i = 0; i < diagram->numsites; ++i )
//         {
//             const jcv_site* site = &sites[i];
//             const jcv_graphedge* e = site->edges;
//             while( e )
//             {
//                 draw_triangle( site->p, e->pos[0], e->pos[1]);
//                 e = e->next;
//             }
//         }
//     }
//     // Here is a simple example of how to do the relaxations of the cells
//     void relax_points(const jcv_diagram* diagram, jcv_point* points)
//     {
//         const jcv_site* sites = jcv_diagraget_sites_(diagram);
//         for( int i = 0; i < diagram->numsites; ++i )
//         {
//             const jcv_site* site = &sites[i];
//             jcv_point sum = site->p;
//             int count = 1;
//             const jcv_graphedge* edge = site->edges;
//             while( edge )
//             {
//                 sum.x += edge->pos[0].x;
//                 sum.y += edge->pos[0].y;
//                 ++count;
//                 edge = edge->next;
//             }
//             points[site->index].x = sum.x / count;
//             points[site->index].y = sum.y / count;
//         }
//     }
//  */
