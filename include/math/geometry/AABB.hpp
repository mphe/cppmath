#ifndef CPPMATH_AABB_HPP
#define CPPMATH_AABB_HPP

#include "Vector.hpp"
#include "Intersection.hpp"

namespace geometry
{
    template <class T>
    class AABB
    {
        public:
            AABB();
            AABB(T x, T y, T w, T h);
            AABB(const Vec2<T>& pos_, const Vec2<T>& size_);

        public:
            // TODO: This functions was useful at some point but probably not
            // anymore. Consider removing it.
            void crop(const AABB<T>& rect);

            bool contains(const Vec2<T>& point) const;
            bool contains(const AABB<T>& rect) const;
            Intersection<T> intersect(const AABB<T>& rect) const;

            bool operator!=(const AABB<T>& r) const;
            bool operator==(const AABB<T>& r) const;

            template <class T2>
            operator AABB<T2>() const;

        public:
            Vec2<T> pos;
            Vec2<T> size;
    };

    typedef AABB<float> AABBf;
    typedef AABB<int> AABBi;
}

#include "AABB.inl"

#endif
