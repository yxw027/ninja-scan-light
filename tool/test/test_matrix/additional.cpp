#include "param/matrix_fixed.h"
#include "param/matrix_special.h"

#include <boost/type_traits/is_same.hpp>

#include <boost/test/unit_test.hpp>
#include "common.h"

using namespace std;

typedef double content_t;
typedef Matrix<content_t> matrix_t;
typedef Matrix<Complex<content_t> > cmatrix_t;

BOOST_FIXTURE_TEST_SUITE(matrix, Fixture<content_t>)

#if !defined(SKIP_FIXED_MATRIX_TESTS) // tests for fixed
BOOST_AUTO_TEST_CASE_MAY_FAILURES(fixed, 1){
  prologue_print();
  typedef /*typename*/ Matrix_Fixed<content_t, SIZE>::mat_t fixed_t;
  fixed_t _A(fixed_t::blank(SIZE, SIZE).replace(*A));
  matrix_compare_delta(*A, _A, ACCEPTABLE_DELTA_DEFAULT);

  typedef /*typename*/ Matrix_Fixed<Complex<content_t>, SIZE>::mat_t cfixed_t;
  cfixed_t _Ac1(cfixed_t::blank(SIZE, SIZE).replace(*A));
  cfixed_t _Ac2(_Ac1.copy());
  matrix_compare_delta(_A, _Ac1, ACCEPTABLE_DELTA_DEFAULT);
  matrix_compare_delta(_A, _Ac2, ACCEPTABLE_DELTA_DEFAULT);

  try{
    matrix_compare_delta(A->eigen(), _A.eigen(), ACCEPTABLE_DELTA_DEFAULT);
    matrix_compare_delta(
        A->partial(SIZE - 1, SIZE - 1, 0, 0).eigen(),
        _A.partial(SIZE - 1, SIZE - 1, 0, 0).eigen(),
        ACCEPTABLE_DELTA_DEFAULT);
  }catch(std::runtime_error &e){
    BOOST_ERROR("eigen_error:" << e.what());
  }
}

BOOST_AUTO_TEST_CASE(fixed_types){
  BOOST_CHECK((boost::is_same<
      Matrix_Frozen<content_t, Array2D_Operator<content_t, Array2D_Operator_Multiply<
        Matrix_Frozen<content_t,Array2D_Fixed<content_t, 2, 4> >,
        Matrix_Frozen<content_t, Array2D_Operator<content_t, Array2D_Operator_Multiply<
          Matrix_Frozen<content_t,Array2D_Fixed<content_t, 4, 8> >,
          Matrix_Frozen<content_t,Array2D_Fixed<content_t, 8, 16> > > > > > > >::builder_t::assignable_t,
      Matrix_Fixed<content_t, 2, 16>::mat_t>::value));
  BOOST_CHECK((boost::is_same<
      Matrix_Frozen<content_t, Array2D_Operator<content_t, Array2D_Operator_Multiply<
        Matrix_Frozen<content_t, Array2D_Operator<content_t, Array2D_Operator_Multiply<
          Matrix_Frozen<content_t,Array2D_Fixed<content_t, 2, 4> >,
          Matrix_Frozen<content_t,Array2D_Fixed<content_t, 4, 8> > > > >,
        Matrix_Frozen<content_t,Array2D_Fixed<content_t, 8, 16> > > > >::builder_t::assignable_t,
      Matrix_Fixed<content_t, 2, 16>::mat_t>::value));
  BOOST_CHECK((boost::is_same<
      Matrix_Fixed<content_t, 2, 4>::mat_t
        ::template Multiply_Matrix_by_Matrix<Matrix_Fixed<content_t, 4, 8>::mat_t::frozen_t>::mat_t
        ::template Multiply_Matrix_by_Matrix<Matrix_Fixed<content_t, 16, 8>::mat_t::frozen_t::builder_t::transpose_t>::mat_t
        ::builder_t::transpose_t::builder_t::assignable_t,
      Matrix_Fixed<content_t, 16, 2>::mat_t>::value));
  BOOST_CHECK((boost::is_same<
      Matrix_Fixed<content_t, 2, 4>::mat_t
        ::template Multiply_Matrix_by_Matrix<Matrix_Fixed<content_t, 4, 8>::mat_t::frozen_t>::mat_t
        ::template Add_Matrix_to_Matrix<Matrix_Fixed<content_t, 3, 7>::mat_t::frozen_t>::mat_t
        ::template Multiply_Matrix_by_Scalar<int>::mat_t
        ::template Multiply_Matrix_by_Matrix<Matrix_Fixed<content_t, 8, 16>::mat_t::frozen_t>::mat_t
        ::builder_t::assignable_t,
      Matrix_Fixed<content_t, 2, 16>::mat_t>::value));
}
#endif

#if !defined(SKIP_SPECIAL_MATRIX_TESTS) // tests for special
BOOST_AUTO_TEST_CASE(force_symmetric){
  assign_linear();
  prologue_print();

  matrix_t A_(as_symmetric(*A));
  BOOST_TEST_MESSAGE("symmetric:" << as_symmetric(*A));
  BOOST_TEST_MESSAGE("symmetric_copy:" << A_);
  matrix_compare(A_, as_symmetric(*A));
  BOOST_CHECK(A->isSymmetric() == false);
  BOOST_CHECK(A_.isSymmetric() == true);

  BOOST_CHECK_THROW(as_symmetric(A->partial(SIZE - 1, SIZE), true), std::runtime_error); // !square

  matrix_inspect_contains(as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) * 2, "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) / 2, "*view: [Symmetric] [Base]");
  matrix_inspect_contains(-as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) + 2, "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) - 2, "*view: [Symmetric] [Base]");
  matrix_inspect_contains(2 * as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(2 + as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(2 - as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) * matrix_t::getI(A->columns()), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) + (*A), "*view: [Base]");
  matrix_inspect_contains(as_symmetric(*A) - (*A), "*view: [Base]");
  matrix_inspect_contains(as_symmetric(*A) * (*A), "*view: [Base]");

  matrix_inspect_contains(as_symmetric(*A).transpose(), "*view: [Symmetric] [Base]"); // should be same after transpose()
  matrix_inspect_contains(as_symmetric(as_symmetric(*A)), "*view: [Symmetric] [Base]"); // as_symmetric should be effective only once
  matrix_inspect_contains(as_symmetric(matrix_t::getI(A->rows())), "*view: [Base]");

#if !defined(SKIP_FIXED_MATRIX_TESTS)
  typedef /*typename*/ Matrix_Fixed<content_t, SIZE>::mat_t fixed_t;
  fixed_t A_fixed(fixed_t::blank(SIZE, SIZE).replace(*A));
  fixed_t A_fixed_(as_symmetric(A_fixed));
  BOOST_TEST_MESSAGE("symmetric_fixed:" << as_symmetric(A_fixed));
  BOOST_TEST_MESSAGE("symmetric_fixed_copy:" << A_fixed_);
  matrix_compare(A_fixed_, as_symmetric(A_fixed));
  BOOST_CHECK(A_fixed.isSymmetric() == false);
  BOOST_CHECK(A_fixed_.isSymmetric() == true);
#endif
}

BOOST_AUTO_TEST_CASE(force_diagonal){
  assign_linear();
  prologue_print();

  matrix_t A_(as_diagonal(*A));
  BOOST_TEST_MESSAGE("diagonal:" << as_diagonal(*A));
  BOOST_TEST_MESSAGE("diagonal_copy:" << A_);
  matrix_compare(A_, as_diagonal(*A));
  BOOST_CHECK(A->isDiagonal() == false);
  BOOST_CHECK(A_.isDiagonal() == true);

  BOOST_CHECK_THROW(as_diagonal(A->partial(SIZE - 1, SIZE), true), std::runtime_error); // !square

  matrix_inspect_contains(as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) * 2, "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) / 2, "*view: [Diagonal] [Base]");
  matrix_inspect_contains(-as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) + 2, "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) - 2, "*view: [Diagonal] [Base]");
  matrix_inspect_contains(2 * as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(2 + as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(2 - as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) * matrix_t::getI(A->columns()), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_diagonal(*A) + (*A), "*view: [Base]");
  matrix_inspect_contains(as_diagonal(*A) - (*A), "*view: [Base]");
  matrix_inspect_contains(as_diagonal(*A) * (*A), "*view: [Base]");

  matrix_inspect_contains(as_diagonal(*A).transpose(), "*view: [Diagonal] [Base]"); // should be same after transpose()
  matrix_inspect_contains(as_diagonal(as_diagonal(*A)), "*view: [Diagonal] [Base]"); // as_diagonal should be effective only once
  matrix_inspect_contains(as_diagonal(matrix_t::getI(A->rows())), "*view: [Base]");

  matrix_compare(A_ * (*A), as_diagonal(*A) * (*A));
  matrix_compare((*A) * A_, (*A) * as_diagonal(*A));
  matrix_compare(A_ * A_, as_diagonal(*A) * as_diagonal(*A));

#if !defined(SKIP_FIXED_MATRIX_TESTS)
  typedef /*typename*/ Matrix_Fixed<content_t, SIZE>::mat_t fixed_t;
  fixed_t A_fixed(fixed_t::blank(SIZE, SIZE).replace(*A));
  fixed_t A_fixed_(as_diagonal(A_fixed));
  BOOST_TEST_MESSAGE("diagonal_fixed:" << as_diagonal(A_fixed));
  BOOST_TEST_MESSAGE("diagonal_fixed_copy:" << A_fixed_);
  matrix_compare(A_fixed_, as_diagonal(A_fixed));
  BOOST_CHECK(A_fixed.isDiagonal() == false);
  BOOST_CHECK(A_fixed_.isDiagonal() == true);
#endif
}

BOOST_AUTO_TEST_CASE(force_special_intersection){
  assign_linear();
  prologue_print();

  matrix_inspect_contains(as_symmetric(*A) + as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) + as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_symmetric(*A) + as_diagonal(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) + as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) - as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) - as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_symmetric(*A) - as_diagonal(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) - as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_symmetric(*A) * as_symmetric(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) * as_diagonal(*A), "*view: [Diagonal] [Base]");
  matrix_inspect_contains(as_symmetric(*A) * as_diagonal(*A), "*view: [Symmetric] [Base]");
  matrix_inspect_contains(as_diagonal(*A) * as_symmetric(*A), "*view: [Symmetric] [Base]");
}

#endif

BOOST_AUTO_TEST_SUITE_END()

