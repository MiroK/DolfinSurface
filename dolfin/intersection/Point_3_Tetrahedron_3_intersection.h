// Copyright (C) 2009 Andre Massing
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// First added:  2009-09-11
// Last changed: 2011-08-09

#ifndef CGAL_POINT_3_TETRAHEDRON_3_INTERSECTION_H
#define CGAL_POINT_3_TETRAHEDRON_3_INTERSECTION_H

#include <CGAL/Point_3.h>
#include <CGAL/Object.h>
#include <CGAL/Tetrahedron_3.h>

CGAL_BEGIN_NAMESPACE

#if CGAL_VERSION_NR < 1030601000
namespace CGALi {
#else
namespace internal {
#endif

  template <class K>
  inline
  bool
  do_intersect(const typename K::Point_3 &pt,
	       const typename K::Tetrahedron_3 &tet,
	       const K&)
  {
    return !tet.has_on_unbounded_side(pt);
  }

  template <class K>
  inline
  bool
  do_intersect(const typename K::Tetrahedron_3 &tet,
	       const typename K::Point_3 &pt,
	       const K&)
  {
    return !tet.has_on_unbounded_side(pt);
  }


  template <class K>
  inline
  Object
  intersection(const typename K::Point_3 &pt,
	       const typename K::Tetrahedron_3 &tet,
	       const K&)
  {
    if (do_intersect(pt,tet))
    {
      return make_object(pt);
    }
    return Object();
  }

  template <class K>
  inline
  Object
  intersection( const typename K::Tetrahedron_3 &tet,
	        const typename K::Point_3 &pt,
	        const K&)
  {
    if (do_intersect(pt,tet))
    {
      return make_object(pt);
    }
    return Object();
  }

} // namespace CGALi


template <class K>
inline bool
do_intersect(const Tetrahedron_3<K> &tet, const Point_3<K> &pt)
{
  typedef typename K::Do_intersect_3 Do_intersect;
  return Do_intersect()(pt, tet);
}

template <class K>
inline bool
do_intersect(const Point_3<K> &pt, const Tetrahedron_3<K> &tet)
{
  typedef typename K::Do_intersect_3 Do_intersect;
  return Do_intersect()(pt, tet);
}


template <class K>
inline Object
intersection(const Tetrahedron_3<K> &tet, const Point_3<K> &pt)
{
  typedef typename K::Intersect_3 Intersect;
  return Intersect()(pt, tet);
}

template <class K>
inline Object
intersection(const Point_3<K> &pt, const Tetrahedron_3<K> &tet)
{
  typedef typename K::Intersect_3 Intersect;
  return Intersect()(pt, tet);
}

CGAL_END_NAMESPACE

#endif