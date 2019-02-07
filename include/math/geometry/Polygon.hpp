#ifndef CPPMATH_BASE_POINT_SET_HPP
#define CPPMATH_BASE_POINT_SET_HPP

#include "Line2.hpp"

namespace math
{
    template <typename T>
    class AbstractPolygon
    {
        public:
            AbstractPolygon();
            AbstractPolygon(bool closed, bool filled, NormalDirection ndir);
            virtual ~AbstractPolygon() {}

            virtual void      add(const Point2<T>& point)          = 0;
            virtual void      edit(size_t i, const Point2<T>& p)   = 0;
            virtual void      insert(size_t i, const Point2<T>& p) = 0;
            virtual void      remove(size_t i)                     = 0;
            virtual void      clear()                              = 0;
            virtual size_t    size() const                         = 0;
            virtual Point2<T> get(size_t i) const                  = 0;
            virtual AABB<T>   getBBox() const                      = 0;
            virtual bool      isConvex() const                     = 0;

            // Return a line segment from point i to point j.
            Line2<T> getSegment(size_t i, size_t j) const;

            // Calls a lambda for each two consecutive points.
            // Returning true breaks the loop.
            // Callback signature: bool (const Line2<T>&)
            template <typename F>
            void foreachSegment(F f) const;

        public:
            bool closed;
            bool filled;
            NormalDirection normaldir;

        protected:
            // NOTE: these are intentionally not the default for the
            //       respective virtual functions as they are not cached
            //       but recalculate on every call.
            AABB<T> _calculateBBox() const;
            bool _calculateConvex() const;
    };


    // TODO: find a better class name
    template <typename T>
    class BasePolygon : public AbstractPolygon<T>
    {
        public:
            BasePolygon();
            BasePolygon(bool closed, bool filled, NormalDirection ndir);
            virtual ~BasePolygon() {};

            void add(const Point2<T>& point)          final;
            void edit(size_t i, const Point2<T>& p)   final;
            void insert(size_t i, const Point2<T>& p) final;
            void remove(size_t i)                     final;
            void clear()                              final;

            virtual AABB<T> getBBox() const     override;
            virtual bool    isConvex() const    override;

        protected:
            // Called whenever the vertex list changed
            virtual void _onVertexChanged() {};

            virtual void _add(const Point2<T>& point)          = 0;
            virtual void _edit(size_t i, const Point2<T>& p)   = 0;
            virtual void _insert(size_t i, const Point2<T>& p) = 0;
            virtual void _remove(size_t i)                     = 0;
            virtual void _clear()                              = 0;

        protected:
            mutable AABB<T> _bbox;
            mutable bool _convex;
            mutable bool _bboxdirty;
            mutable bool _convexdirty;
    };
}


#include "../math.hpp"
#include "intersect.hpp"

// Implementation
namespace math
{
    // AbstractPolygon
    template <typename T>
    AbstractPolygon<T>::AbstractPolygon() :
        AbstractPolygon(true, true, NormalBoth)
    { }

    template <typename T>
    AbstractPolygon<T>::AbstractPolygon(bool closed_, bool filled_, NormalDirection ndir) :
        closed(closed_),
        filled(filled_),
        normaldir(ndir)
    { }

    template <typename T>
    Line2<T> AbstractPolygon<T>::getSegment(size_t i, size_t j) const
    {
        return Line2<T>(get(i), get(j), Segment);
    }

    template <typename T>
    template <typename F>
    void AbstractPolygon<T>::foreachSegment(F f) const
    {
        for (size_t i = 1; i < size(); ++i)
            if (f(getSegment(i - 1, i)))
                return;
        if (closed && size() > 2)
            f(getSegment(size() - 1, 0));
    }

    template <typename T>
    AABB<T> AbstractPolygon<T>::_calculateBBox() const
    {
        if (this->size() < 2)
        {
            return AABB<T>();
        }

        Vec2f min = this->get(0).asVector(),
              max = this->get(0).asVector();

        for (size_t i = 1; i < this->size(); ++i)
        {
            auto p = this->get(i);
            for (int k = 0; k < 2; ++k)
            {
                min[k] = std::min(min[k], p[k]);
                max[k] = std::max(max[k], p[k]);
            }
        }

        return AABB<T>(min.asPoint(), max - min);
    }

    template <typename T>
    bool AbstractPolygon<T>::_calculateConvex() const
    {
        if (this->size() <= 3)
            return true;
        else
        {
            int sign;
            for (size_t i = 0; i < this->size(); ++i)
            {
                auto a = this->get(math::wrap(i + 1, this->size())) - this->get(i);
                auto b = this->get(math::wrap(i + 2, this->size())) - this->get(i);

                int newsign = math::sign(a.cross(b));
                if (i == 0)
                    sign = newsign;
                else if (newsign != 0 && sign != newsign)
                    return false;
            }

            return true;
        }
    }




    // BasePolygon
    template <typename T>
    BasePolygon<T>::BasePolygon() :
        _convex(false),
        _bboxdirty(true),
        _convexdirty(true)
        // NOTE: BBox and convexity should recalculate because it doesn't
        //       know if derived classes automatically add some vertices.
    { }

    // BasePolygon
    template <typename T>
    BasePolygon<T>::BasePolygon(bool closed, bool filled, NormalDirection ndir) :
        AbstractPolygon<T>(closed, filled, ndir),
        _convex(false),
        _bboxdirty(true),
        _convexdirty(true)
        // NOTE: BBox and convexity should recalculate because it doesn't
        //       know if derived classes automatically add some vertices.
    { }

    template <typename T>
    void BasePolygon<T>::add(const Point2<T>& point)
    {
        _add(point);
        if (!intersect(_bbox, this->get(this->size() - 1)))
            _bboxdirty = true;
        _convexdirty = true;
        _onVertexChanged();
    }

    template <typename T>
    void BasePolygon<T>::edit(size_t i, const Point2<T>& p)
    {
        _edit(i, p);
        _bboxdirty = true;
        _convexdirty = true;
        _onVertexChanged();
    }

    template <typename T>
    void BasePolygon<T>::insert(size_t i, const Point2<T>& p)
    {
        _insert(i, p);
        if (!intersect(_bbox, this->get(i)))
            _bboxdirty = true;
        _convexdirty = true;
        _onVertexChanged();
    }

    template <typename T>
    void BasePolygon<T>::remove(size_t i)
    {
        _remove(i);
        _bboxdirty = true;
        _convexdirty = true;
        _onVertexChanged();
    }


    template <typename T>
    void BasePolygon<T>::clear()
    {
        _clear();
        _bboxdirty = true;
        _convexdirty = true;
        _onVertexChanged();
    }

    template <typename T>
    AABB<T> BasePolygon<T>::getBBox() const
    {
        if (_bboxdirty)
        {
            _bbox = this->_calculateBBox();
            _bboxdirty = false;
        }
        return _bbox;
    }

    template <typename T>
    bool BasePolygon<T>::isConvex() const
    {
        if (_convexdirty)
        {
            _convex = this->_calculateConvex();
            _convexdirty = false;
        }
        return _convex;
    }
}
#endif
