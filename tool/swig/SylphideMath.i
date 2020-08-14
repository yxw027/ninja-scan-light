/**
 * @file SWIG interface file for header files in param directory
 *
 */

%module SylphideMath

#define ENABLE_IOSTREAM 1

%{
#include <sstream>
#include <vector>

#if defined(SWIGRUBY) && defined(isfinite)
#undef isfinite_
#undef isfinite
#endif

#include "param/complex.h"
#include "param/matrix.h"

#if defined(SWIGRUBY) && defined(isfinite_)
#undef isfinite_
#define isfinite(x) finite(x)
#endif
%}

%include std_string.i
%include std_vector.i
%include exception.i

%exception {
  try {
    $action
  } catch (const std::exception& e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}

%define STR(x)
#x
%enddef
%define MAKE_SETTER(name, type)
%rename(STR(name ## =)) set_ ## name;
type set_ ## name (const type &v) {
  return (self->name() = v);
}
%enddef
%define MAKE_GETTER(name, type)
%rename(STR(name)) get_ ## name;
type get_ ## name () {
  return self->name();
}
%enddef
%define MAKE_TO_S(type)
%extend type{
  std::string __str__() const {
    std::stringstream s;
    s << (*self);
    return s.str();
  }
};
%enddef

%feature("autodoc", "1");

%ignore Complex::real;
%ignore Complex::imaginary;
%ignore operator<<(std::ostream &, const Complex &);

%include param/complex.h

MAKE_TO_S(Complex);

%extend Complex {
  MAKE_SETTER(real, FloatT);
  MAKE_GETTER(real, FloatT);
  MAKE_SETTER(imaginary, FloatT);
  MAKE_GETTER(imaginary, FloatT);
  %alias power "**"
  %alias arg "angle,phase"
  %alias conjugate "conj"
  // fdiv // TODO
  // finite? // TODO
  %alias set_imaginary "imag=";
  %alias get_imaginary "imag";
  // infinite? // TODO
  %alias abs "magnitude"
  // polar // TOOO
  // rect,rectangle // TODO
};

%define INSTANTIATE_COMPLEX(type, suffix)
%template(Complex ## suffix) Complex<type>;
%enddef

INSTANTIATE_COMPLEX(double, Complex);

template <class T, class Array2D_Type, class ViewType = MatrixViewBase<> >
class Matrix_Frozen {
  protected:
    Matrix_Frozen();
  public:
    const unsigned int rows();
    const unsigned int columns();
    
    typedef Matrix_Frozen<T, Array2D_ScaledUnit<T> > scalar_matrix_t;
    static scalar_matrix_t getScalar(const unsigned int &size, const T &scalar);
    static scalar_matrix_t getI(const unsigned int &size);
    
    template <class T2, class Array2D_Type2, class ViewType2>
    bool operator==(const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const noexcept;
    // bool operator!= // automatically defined
    
    bool isSquare() const noexcept;
    bool isDiagonal() const noexcept;
    bool isSymmetric() const noexcept;
    
    T trace(const bool &do_check = true) const;
    
    // bool isLU() const noexcept
    
    // transpose, ...
};

template <class T, class Array2D_Type, class ViewType = MatrixViewBase<> >
class Matrix : public Matrix_Frozen<T, Array2D_Type, ViewType> {
  public:
    Matrix(const unsigned int &rows, const unsigned int &columns);
};

%define INSTANTIATE_MATRIX_FUNC(func_orig, func_new)
%template(func_new) func_orig<T, Array2D_Dense<T>, ViewType>;
%template(func_new) func_orig<T, Array2D_ScaledUnit<T>, ViewType>;
%enddef

%extend Matrix_Frozen {
  T __getitem__(const unsigned int &row, const unsigned int &column) const {
    return ($self)->operator()(row, column);
  }
  
  INSTANTIATE_MATRIX_FUNC(operator==, __eq__);

  template <class T2, class Array2D_Type2, class ViewType2>
  bool isDifferentSize(const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const {
    return ($self)->isDifferentSize(matrix);
  }
  INSTANTIATE_MATRIX_FUNC(isDifferentSize, isDifferentSize);
  
  Matrix<T, Array2D_Dense<T> > operator*(const T &scalar) const noexcept {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator*(scalar));
  }
  Matrix<T, Array2D_Dense<T> > operator/(const T &scalar) const noexcept {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator/(scalar));
  }
  Matrix<T, Array2D_Dense<T> > operator-() const noexcept {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator-());
  }
  
  template <class T2, class Array2D_Type2, class ViewType2>
  Matrix<T, Array2D_Dense<T> > operator+(
      const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator+(matrix));
  }
  INSTANTIATE_MATRIX_FUNC(operator+, __add__);
  Matrix<T, Array2D_Dense<T> > operator+(const T &scalar) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator+(scalar));
  }
  
  template <class T2, class Array2D_Type2, class ViewType2>
  Matrix<T, Array2D_Dense<T> > operator-(
      const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator-(matrix));
  }
  INSTANTIATE_MATRIX_FUNC(operator-, __sub__);
  Matrix<T, Array2D_Dense<T> > operator-(const T &scalar) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator-(scalar));
  }
  
  template <class T2, class Array2D_Type2, class ViewType2>
  Matrix<T, Array2D_Dense<T> > operator*(
      const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator*(matrix));
  }
  INSTANTIATE_MATRIX_FUNC(operator*, __mul__);
  
  Matrix<T, Array2D_Dense<T> > inverse() const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->inverse());
  }
  template <class T2, class Array2D_Type2, class ViewType2>
  Matrix<T, Array2D_Dense<T> > operator/(
      const Matrix_Frozen<T2, Array2D_Type2, ViewType2> &matrix) const {
    return (Matrix<T, Array2D_Dense<T> >)(($self)->operator/(matrix));
  }
  INSTANTIATE_MATRIX_FUNC(operator/, __div__);
  
  std::string debug() const {
    std::stringstream s;
    s << $self->inspect();
    return s.str();
  }

#ifdef SWIGRUBY
  %rename("is_square?") isSquare;
  %rename("is_diagonal?") isDiagonal;
  %rename("is_symmetric?") isSymmetric;
  %rename("is_different_size?") isDifferentSize;
#endif
};

MAKE_TO_S(Matrix_Frozen)

%extend Matrix {
  T &__setitem__(const unsigned int &row, const unsigned int &column, const T &value) {
    return (($self)->operator()(row, column) = value);
  }
};

%define INSTANTIATE_MATRIX(type, suffix)
%extend Matrix_Frozen<type, Array2D_ScaledUnit<type > > {
  Matrix_Frozen<type, Array2D_ScaledUnit<type > > inverse() const {
    return ($self)->inverse();
  }
};
%template(Matrix_Scalar ## suffix) Matrix_Frozen<type, Array2D_ScaledUnit<type > >;
%template(Matrix_Frozen ## suffix) Matrix_Frozen<type, Array2D_Dense<type > >;
%template(Matrix ## suffix) Matrix<type, Array2D_Dense<type > >;
%enddef

INSTANTIATE_MATRIX(double, D);

