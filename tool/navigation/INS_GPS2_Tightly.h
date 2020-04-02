/*
 *  INS_GPS2_Tightly.h, header file to perform calculation of tightly integration of INS and GPS.
 *  Copyright (C) 2018 M.Naruoka (fenrir)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INS_GPS2_TIGHTLY_H__
#define __INS_GPS2_TIGHTLY_H__

#include <cmath>
#include <iostream>
#include <map>

#include "INS.h"
#include "Filtered_INS2.h"
#include "INS_GPS2.h"
#include "GPS_Solver.h"
#include "coordinate.h"

template <typename BaseINS, unsigned int Clocks>
class INS_ClockErrorEstimated;

template <class BaseINS, unsigned int Clocks>
struct INS_Property<INS_ClockErrorEstimated<BaseINS, Clocks> > {
  static const unsigned CLOCKS_SUPPORTED = Clocks;
  static const unsigned STATE_VALUES_WITHOUT_CLOCK_ERROR = INS_Property<BaseINS>::STATE_VALUES;
  static const unsigned STATE_VALUES_CLOCK_ERROR = 2 * CLOCKS_SUPPORTED;
  static const unsigned STATE_VALUES = STATE_VALUES_WITHOUT_CLOCK_ERROR + STATE_VALUES_CLOCK_ERROR;
};

template <
    typename BaseINS = INS<>, unsigned int Clocks = 1>
class INS_ClockErrorEstimated : public BaseINS {
  public:
#if defined(__GNUC__) && (__GNUC__ < 5)
    typedef typename BaseINS::float_t float_t;
    typedef typename BaseINS::vec3_t vec3_t;
#else
    using typename BaseINS::float_t;
    using typename BaseINS::vec3_t;
#endif

  public:
    static const unsigned CLOCKS_SUPPORTED
        = INS_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::CLOCKS_SUPPORTED;
    static const unsigned STATE_VALUES_WITHOUT_CLOCK_ERROR
        = INS_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::STATE_VALUES_WITHOUT_CLOCK_ERROR;
    static const unsigned STATE_VALUES_CLOCK_ERROR
        = INS_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::STATE_VALUES_CLOCK_ERROR;
    static const unsigned STATE_VALUES
        = INS_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::STATE_VALUES; ///< Number of state values
    virtual unsigned state_values() const {return STATE_VALUES;}

  protected:
    float_t m_clock_error[CLOCKS_SUPPORTED];  ///< receiver clock error [m]
    float_t m_clock_error_rate[CLOCKS_SUPPORTED];  ///< receiver clock error rate [m/s]

  public:
    INS_ClockErrorEstimated()
        : BaseINS() {
      for(unsigned int i(0); i < CLOCKS_SUPPORTED; ++i){
        m_clock_error[i] = 0;
        m_clock_error_rate[i] = 0;
      }
    }

    INS_ClockErrorEstimated(const INS_ClockErrorEstimated &orig, const bool &deepcopy = false)
        : BaseINS(orig, deepcopy) {
      for(unsigned int i(0); i < CLOCKS_SUPPORTED; ++i){
        m_clock_error[i] = orig.m_clock_error[i];
        m_clock_error_rate[i] = orig.m_clock_error_rate[i];
      }
    }

    virtual ~INS_ClockErrorEstimated(){}

    float_t &clock_error(const unsigned int &index = 0){return m_clock_error[index];}
    float_t &clock_error_rate(const unsigned int &index = 0){return m_clock_error_rate[index];}

    using BaseINS::operator[];

    const float_t &operator[](const unsigned &index) const {
      int index_offset(index - STATE_VALUES_WITHOUT_CLOCK_ERROR);
      if((index_offset >= 0) && (index_offset < STATE_VALUES_CLOCK_ERROR)){
        int index_clock(index_offset >> 1);
        return (index_offset % 2 == 0) ? m_clock_error[index_clock] : m_clock_error_rate[index_clock];
      }
      return BaseINS::operator[](index);
    }

    void update(
        const vec3_t &accel, const vec3_t &gyro,
        const float_t &deltaT){
      for(unsigned int i(0); i < CLOCKS_SUPPORTED; ++i){
        m_clock_error[i] += m_clock_error_rate[i] * deltaT;
      }
      BaseINS::update(accel, gyro, deltaT);
    }
};

template <class BaseINS, unsigned int Clocks>
class Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >
    : public Filtered_INS2_Property<BaseINS> {
  public:
    static const unsigned P_SIZE_WITHOUT_CLOCK_ERROR
#if defined(_MSC_VER)
        = Filtered_INS2_Property<BaseINS>::P_SIZE
#endif
        ;
    static const unsigned Q_SIZE_WITHOUT_CLOCK_ERROR
#if defined(_MSC_VER)
        = Filtered_INS2_Property<BaseINS>::Q_SIZE
#endif
        ;
    static const unsigned P_SIZE_CLOCK_ERROR
#if defined(_MSC_VER)
        = INS_ClockErrorEstimated<BaseINS, Clocks>::STATE_VALUES_CLOCK_ERROR
#endif
        ;
    static const unsigned Q_SIZE_CLOCK_ERROR
#if defined(_MSC_VER)
        = INS_ClockErrorEstimated<BaseINS, Clocks>::STATE_VALUES_CLOCK_ERROR
#endif
        ;
    static const unsigned P_SIZE
#if defined(_MSC_VER)
        = P_SIZE_WITHOUT_CLOCK_ERROR + P_SIZE_CLOCK_ERROR
#endif
        ;
    static const unsigned Q_SIZE
#if defined(_MSC_VER)
        = Q_SIZE_WITHOUT_CLOCK_ERROR + Q_SIZE_CLOCK_ERROR
#endif
        ;
};

#if !defined(_MSC_VER)
template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::P_SIZE_WITHOUT_CLOCK_ERROR
    = Filtered_INS2_Property<BaseINS>::P_SIZE;

template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::Q_SIZE_WITHOUT_CLOCK_ERROR
    = Filtered_INS2_Property<BaseINS>::Q_SIZE;

template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::P_SIZE_CLOCK_ERROR
    = INS_ClockErrorEstimated<BaseINS>::STATE_VALUES_CLOCK_ERROR;

template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::Q_SIZE_CLOCK_ERROR
    = INS_ClockErrorEstimated<BaseINS>::STATE_VALUES_CLOCK_ERROR;

template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::P_SIZE
    = P_SIZE_WITHOUT_CLOCK_ERROR + P_SIZE_CLOCK_ERROR;

template <class BaseINS, unsigned int Clocks>
const unsigned Filtered_INS2_Property<INS_ClockErrorEstimated<BaseINS, Clocks> >::Q_SIZE
    = Q_SIZE_WITHOUT_CLOCK_ERROR + Q_SIZE_CLOCK_ERROR;
#endif

template <
    class BaseFINS = Filtered_INS2<INS_ClockErrorEstimated<INS<> > > >
class Filtered_INS_ClockErrorEstimated : public BaseFINS {
  public:
#if defined(__GNUC__) && (__GNUC__ < 5)
    typedef typename BaseFINS::float_t float_t;
    typedef typename BaseFINS::vec3_t vec3_t;
    typedef typename BaseFINS::mat_t mat_t;
#else
    using typename BaseFINS::float_t;
    using typename BaseFINS::vec3_t;
    using typename BaseFINS::mat_t;
#endif

  protected:
    float_t m_beta_clock_error;
    float_t m_beta_clock_error_rate;

  public:
    using BaseFINS::ins_t::CLOCKS_SUPPORTED;
    using BaseFINS::ins_t::STATE_VALUES_WITHOUT_CLOCK_ERROR;
    using BaseFINS::ins_t::STATE_VALUES_CLOCK_ERROR;
    using BaseFINS::property_t::P_SIZE_WITHOUT_CLOCK_ERROR;
    using BaseFINS::property_t::Q_SIZE_WITHOUT_CLOCK_ERROR;
    using BaseFINS::property_t::P_SIZE_CLOCK_ERROR;
    using BaseFINS::property_t::Q_SIZE_CLOCK_ERROR;
    using BaseFINS::m_clock_error;

    Filtered_INS_ClockErrorEstimated()
        : BaseFINS(),
        m_beta_clock_error(1), m_beta_clock_error_rate(1) {

    }

    Filtered_INS_ClockErrorEstimated(
        const Filtered_INS_ClockErrorEstimated &orig,
        const bool &deepcopy = false)
        : BaseFINS(orig, deepcopy),
        m_beta_clock_error(orig.m_beta_clock_error),
        m_beta_clock_error_rate(orig.m_beta_clock_error_rate) {

    }

    ~Filtered_INS_ClockErrorEstimated(){}

    float_t &beta_clock_error(){return m_beta_clock_error;}
    float_t &beta_clock_error_rate(){return m_beta_clock_error_rate;}

    void getAB(
        const vec3_t &accel,
        const vec3_t &gyro,
        typename BaseFINS::getAB_res &res) const {

      BaseFINS::getAB(accel, gyro, res);

      { // A matrix modification
        // Modify A associated with clock error, then, rate
        for(unsigned i(P_SIZE_WITHOUT_CLOCK_ERROR), j(0);
            j < CLOCKS_SUPPORTED; i += 2, j++){

          /* Matrix layout (A)
           * [-b_c] [      1] : row(j*2)   => clock(j) error
           * [   0] [-b_cdot] : row(j*2+1) => clock(j) error rate
           */
          res.A[i][i] += (-m_beta_clock_error);
          res.A[i][i + 1] += 1; // d(clock_error)/dt = clock_error_rate
          res.A[i + 1][i + 1] += (-m_beta_clock_error_rate);
        }
      }

      { // B matrix modification
        for(unsigned i(P_SIZE_WITHOUT_CLOCK_ERROR), j(Q_SIZE_WITHOUT_CLOCK_ERROR), k(0);
             k < Q_SIZE_CLOCK_ERROR;
             i++, j++, k++){

          /* Matrix layout (B)
           * [1] : row(j*2)   => clock(j) error
           * [1] : row(j*2+1) => clock(j) error rate
           */
          res.B[i][j] += 1;
        }
      }
    }

    /**
     * Time update
     *
     * @param accel acceleration
     * @param gyro angular speed
     * @param deltaT delta T
     */
    void update(const vec3_t &accel, const vec3_t &gyro, const float_t &deltaT){
      BaseFINS::update(accel, gyro, deltaT);
    }

    /**
     * Kalman By using correction values @f$ \Hat{x} @f$ estimated by filter, correct INS.
     *
     * @param x_hat correction values estimated by Kalman Filter
     */
    void correct_INS(mat_t &x_hat){
      for(unsigned i(P_SIZE_WITHOUT_CLOCK_ERROR), j(STATE_VALUES_WITHOUT_CLOCK_ERROR), k(0);
          k < STATE_VALUES_CLOCK_ERROR;
          i++, j++, k++){
        (*this)[j] -= x_hat(i, 0);
      }
      BaseFINS::correct_INS(x_hat);
    }

    using BaseFINS::correct_primitive;

    /**
     * Measurement update
     *
     * @param H Observation matrix
     * @param z Observation values
     * @param R Error covariance of z
     */
    void correct_primitive(const mat_t &H, const mat_t &z, const mat_t &R){
      BaseFINS::correct_primitive(H, z, R);
    }
};

template <class FloatT>
struct GPS_RawData {
  typedef GPS_SinglePositioning<FloatT> solver_t;
  const solver_t *solver;

  unsigned int clock_index;

  typedef int prn_t;
  typedef std::vector<std::pair<prn_t, FloatT> > prn_obs_t;

  enum measurement_items_t {
    L1_PSEUDORANGE,
    L1_DOPPLER,
    L1_CARRIER_PHASE,
    L1_RANGE_RATE,
    MEASUREMENT_ITEMS_PREDEFINED,
  };
  typedef std::map<prn_t, std::map<int, FloatT> > measurement_t;
  measurement_t measurement;

  prn_obs_t measurement_of(
      const typename measurement_t::mapped_type::key_type &key,
      const FloatT &scaling = FloatT(1)) const {
    prn_obs_t res;
    for(typename measurement_t::const_iterator it(measurement.begin());
        it != measurement.end(); ++it){
      typename measurement_t::mapped_type::const_iterator it2(it->second.find(key));
      if(it2 == it->second.end()){continue;}
      res.push_back(std::make_pair(it->first, it2->second * scaling));
    }
    return res;
  }

  static prn_obs_t difference(
      const prn_obs_t &operand, const prn_obs_t &argument,
      const FloatT &scaling = FloatT(1)) {
    prn_obs_t res;
    for(typename prn_obs_t::const_iterator it(operand.begin()); it != operand.end(); ++it){
      for(typename prn_obs_t::const_iterator it2(argument.begin()); it2 != argument.end(); ++it2){
        if(it->first != it2->first){continue;}
        res.push_back(std::make_pair(it->first, (it->second - it2->second) * scaling));
        break;
      }
    }
    return res;
  }

  typedef GPS_Time<FloatT> gps_time_t;
  gps_time_t gpstime;

  GPS_RawData(const unsigned int &_clock_index = 0)
      : solver(NULL),
      clock_index(_clock_index), measurement(), gpstime() {}
  ~GPS_RawData(){}
};

/**
 * @brief Tightly coupled INS/GPS
 *
 * Tightly coupled INS/GPS
 *
 * @param BaseFINS Filtered INS, which mainly supports for time update procedure.
 * @see Filtered_INS2
 */
template <
  class BaseFINS = Filtered_INS_ClockErrorEstimated<Filtered_INS2<
      INS_ClockErrorEstimated<INS<> > > >
>
class INS_GPS2_Tightly : public BaseFINS{
  public:
#if defined(__GNUC__) && (__GNUC__ < 5)
    typedef typename BaseFINS::float_t float_t;
    typedef typename BaseFINS::vec3_t vec3_t;
    typedef typename BaseFINS::quat_t quat_t;
    typedef typename BaseFINS::mat_t mat_t;
#else
    using typename BaseFINS::float_t;
    using typename BaseFINS::vec3_t;
    using typename BaseFINS::quat_t;
    using typename BaseFINS::mat_t;
#endif
  public:
    INS_GPS2_Tightly() : BaseFINS() {}

    INS_GPS2_Tightly(const INS_GPS2_Tightly &orig, const bool &deepcopy = false)
        : BaseFINS(orig, deepcopy) {
    }

    ~INS_GPS2_Tightly(){}
    
    typedef INS_GPS2_Tightly<BaseFINS> self_t;

    typedef GPS_RawData<float_t> raw_data_t;
    typedef GPS_SpaceNode<float_t> space_node_t;
    typedef typename raw_data_t::solver_t solver_t;

    using BaseFINS::CLOCKS_SUPPORTED;
    using BaseFINS::P_SIZE;
    using BaseFINS::property_t::P_SIZE_WITHOUT_CLOCK_ERROR;

  protected:
    struct receiver_state_t {
      typename raw_data_t::gps_time_t t;
      unsigned int clock_index;
      float_t clock_error;
      typename solver_t::pos_t pos;
      typename solver_t::xyz_t vel;
    };
    receiver_state_t receiver_state(
        const typename raw_data_t::gps_time_t &t,
        const unsigned int &clock_index,
        const float_t &clock_error_shift = 0) const {
      float_t clock_error(
          BaseFINS::m_clock_error[clock_index] + clock_error_shift);
      receiver_state_t res = {
        t - clock_error / space_node_t::light_speed,
        clock_index,
        clock_error,
        {
          BaseFINS::template position_xyz<typename solver_t::xyz_t>(),
          typename solver_t::llh_t(BaseFINS::phi, BaseFINS::lambda, BaseFINS::h),
        },
        BaseFINS::template velocity_xyz<typename solver_t::xyz_t>(),
      };
      return res;
    }

    /**
     * Assign items of z, H and R of Kalman filter matrices based on range and rate residuals
     *
     * @param solver residual calculator
     * @param prn GNSS satellite number used as target
     * @param x receiver state represented by current position and clock properties
     * @param range Measured range including all error including user and satellite clock errors
     * @param rate pointer of measured rate, if unavailable, NULL can be specified
     * @param z (output) pointer to be stored with residual
     * @param H (output) pointer to be stored with correlation of state
     * @param R_diag (output) pointer to be stored with estimated residual variance
     * @return (int) number of used rows
     */
    int assign_z_H_R(
        const solver_t &solver,
        const typename raw_data_t::prn_t &prn,
        const receiver_state_t &x,
        const float_t &range, const float_t *rate,
        float_t z[], float_t H[][P_SIZE], float_t R_diag[]) const {

      typename solver_t::relative_property_t prop(
          solver.relative_property(prn, range - x.clock_error, x.t, x.pos, x.vel));

      if(prop.weight <= 0){return 0;} // Intentional exclusion

      z[0] = prop.range_residual;

      for(int i(0); i < (rate ? 2 : 1); ++i){ // zero clear
        for(int j(0); j < P_SIZE; ++j){
          H[i][j] = 0;
        }
      }

      { // setup H matrix
#define pow2(x) ((x) * (x))
#define q_e2n(i) BaseFINS::q_e2n.get(i)
        float_t H_uh[3][4] = {{0}};
        const float_t
            q_alpha((pow2(q_e2n(0)) + pow2(q_e2n(3))) * 2 - 1),
            q_beta((q_e2n(0) * q_e2n(1) - q_e2n(2) * q_e2n(3)) * 2),
            q_gamma((q_e2n(0) * q_e2n(2) + q_e2n(1) * q_e2n(3)) * 2);
        static const float_t &e(BaseFINS::Earth::epsilon_Earth);
        const float_t n(BaseFINS::Earth::R_e / std::sqrt(1.0 - pow2(e * q_alpha)));
        const float_t sf(n * pow2(e) * q_alpha * -2 / (1.0 - pow2(e) * pow2(q_alpha)));
        const float_t n_h((n + BaseFINS::h) * 2);
#undef q_e2n
        H_uh[0][0] = -q_gamma * q_beta * sf;
        H_uh[0][1] = -pow2(q_gamma) * sf - n_h * q_alpha;
        H_uh[0][2] = -n_h * q_beta;
        H_uh[0][3] = -q_gamma;

        H_uh[1][0] = pow2(q_beta) * sf + n_h * q_alpha;
        H_uh[1][1] = q_beta * q_gamma * sf;
        H_uh[1][2] = -n_h * q_gamma;
        H_uh[1][3] = q_beta;

        {
          const float_t sf2(sf * -(1.0 - pow2(e)));
          const float_t n_h2((n * (1.0 - pow2(e)) + BaseFINS::h) * 2);
          H_uh[2][0] = q_alpha * q_beta * sf2 + n_h2 * q_beta;
          H_uh[2][1] = q_alpha * q_gamma * sf2 + n_h2 * q_gamma;
          H_uh[2][3] = -q_alpha;
        }
#undef pow2

        for(int j(0), k(3); j < sizeof(H_uh[0]) / sizeof(H_uh[0][0]); ++j, ++k){
          for(int i(0); i < sizeof(prop.los_neg) / sizeof(prop.los_neg[0]); ++i){
            H[0][k] -= prop.los_neg[i] * H_uh[i][j]; // polarity checked.
          }
        }
        H[0][P_SIZE_WITHOUT_CLOCK_ERROR + (x.clock_index * 2)] = -1; // polarity checked.
      }

      if(prop.weight < 1E-1){prop.weight = 1E-1;}
      R_diag[0] = std::pow(1.0 / prop.weight, 2); // TODO range error variance [m]

      if(!rate){return 1;}

      // rate residual
      z[1] = *rate - BaseFINS::m_clock_error_rate[x.clock_index]
          + prop.rate_relative_neg;

      { // setup H matrix
        { // velocity
          mat_t dcm_q_e2n_star(BaseFINS::q_e2n.conj().getDCM());
          for(int j(0); j < dcm_q_e2n_star.columns(); ++j){
            for(int i(0); i < sizeof(prop.los_neg) / sizeof(prop.los_neg[0]); ++i){
              H[1][j] -= prop.los_neg[i] * dcm_q_e2n_star(i, j); // polarity checked.
            }
          }
        }
        { // position
          const float_t &vx(x.vel.x()), &vy(x.vel.y()), &vz(x.vel.z());
          H[1][3] -= (                        prop.los_neg[1] * -vz + prop.los_neg[2] *  vy) * 2;  // polarity checked.
          H[1][4] -= (prop.los_neg[0] *  vz                         + prop.los_neg[2] * -vx) * 2;
          H[1][5] -= (prop.los_neg[0] * -vy + prop.los_neg[1] *  vx                        ) * 2;
        }
        // error rate
        H[1][P_SIZE_WITHOUT_CLOCK_ERROR + (x.clock_index * 2) + 1] = -1; // polarity checked.
      }

      R_diag[1] = R_diag[0] * 1E-3; // TODO rate error variance

      return 2;
    }

  public:
    /**
     * Calculate information required for measurement update
     *
     * @param gps GPS observation mainly consisting of range (and rate, if available)
     * @param clock_error_shift forcefully shifting value of clock error in meter,
     * which will be used when receiver clock error exceeds predefined threshold
     * of allowable delta from true GPS time. Normally it is +/- (1 ms * speed of light).
     */
    CorrectInfo<float_t> correct_info(
        const raw_data_t &gps,
        const float_t &clock_error_shift = 0) const {

      if(gps.clock_index >= CLOCKS_SUPPORTED){return CorrectInfo<float_t>::no_info();}

      // check space_node is configured
      if(!gps.solver){return CorrectInfo<float_t>::no_info();}

      receiver_state_t x(receiver_state(gps.gpstime, gps.clock_index, clock_error_shift));

      struct buf_t {
        float_t *z;
        float_t (*H)[P_SIZE];
        float_t *R_diag;
        buf_t(const unsigned int &size)
            : z(new float_t [size * (P_SIZE + 2)]),
            R_diag(&z[size]), H((float_t (*)[P_SIZE])&z[size * 2]) {}
        ~buf_t(){
          delete [] z;
        }
      } buf(gps.measurement.size() * 2); // range + rate

      // count up valid measurement, and make observation matrices
      int z_index(0);

      typedef typename raw_data_t::measurement_t::const_iterator it_t;
      typedef typename raw_data_t::measurement_t::mapped_type::const_iterator it2_t;

      for(it_t it_sat(gps.measurement.begin()); it_sat != gps.measurement.end(); ++it_sat){
        typename raw_data_t::prn_t prn(it_sat->first);

        it2_t it2_range(it_sat->second.find(raw_data_t::L1_PSEUDORANGE));
        if(it2_range == it_sat->second.end()){ // No range information
          continue;
        }

        float_t rate;
        const float_t *rate_p(NULL);
        it2_t it2_rate(it_sat->second.find(raw_data_t::L1_RANGE_RATE));
        if(it2_rate != it_sat->second.end()){
          rate_p = &(it2_rate->second);
        }else{ // fall back by using doppler
          it2_t it2_doppler(it_sat->second.find(raw_data_t::L1_DOPPLER));
          if(it2_doppler != it_sat->second.end()){
            // calculate range rate derived from doppler
            rate = -(it2_doppler->second) * space_node_t::L1_WaveLength();
            rate_p = &rate;
          }
        }

        /* Intentional exclusion, in which zero will be returned,
         * may be occurred during residual calculation such as elevation mask.
         */
        z_index += assign_z_H_R(*gps.solver,
            prn, x,
            it2_range->second, rate_p,
            &buf.z[z_index], &buf.H[z_index], &buf.R_diag[z_index]);
      }

      if(z_index <= 0){return CorrectInfo<float_t>::no_info();}

      mat_t H(z_index, P_SIZE, (float_t *)buf.H);
      mat_t z(z_index, 1, buf.z);
      mat_t R(z_index, z_index);
      for(int i(0); i < z_index; ++i){
        R(i, i) = buf.R_diag[i];
      }

      return CorrectInfo<float_t>(H, z, R);
    }

    CorrectInfo<float_t> correct_info(
        const raw_data_t &gps,
        const vec3_t &lever_arm_b, const vec3_t &omega_b2i_4b,
        const float_t &clock_error_shift = 0) const {
      // TODO
      return correct_info(gps, clock_error_shift);
    }

  protected:
    float_t range_residual_mean_ms(
        const unsigned int &clock_index,
        const CorrectInfo<float_t> &info) const {

      float_t range_residual_sum(0);
      unsigned int z_ranges(0);

      for(int i(0); i < info.z.rows(); i++){
        if(info.H(i, P_SIZE_WITHOUT_CLOCK_ERROR + (clock_index * 2)) > -0.5){continue;}
        range_residual_sum += info.z(i, 0);
        z_ranges++;
      }

      return range_residual_sum / z_ranges / space_node_t::light_speed / 1E-3;
    }

    struct CorrectInfoGenerator1 {
      CorrectInfo<float_t> operator()(
          const self_t &self, const raw_data_t &gps,
          const float_t &clock_error_shift = 0) const {
        return self.correct_info(gps, clock_error_shift);
      }
    };

    struct CorrectInfoGenerator2 {
      const vec3_t &lever_arm_b;
      const vec3_t &omega_b2i_4b;
      CorrectInfoGenerator2(const vec3_t &lever, const vec3_t &omega)
          : lever_arm_b(lever), omega_b2i_4b(omega) {}
      CorrectInfo<float_t> operator()(
          const self_t &self, const raw_data_t &gps,
          const float_t &clock_error_shift = 0) const {
        return self.correct_info(gps, lever_arm_b, omega_b2i_4b, clock_error_shift);
      }
    };

    template <class Generator>
    void correct_generic(const raw_data_t &gps, const Generator &generator){
      CorrectInfo<float_t> info(generator(*this, gps));
      if(info.z.rows() < 1){return;}

      /*
       * detection for automatic receiver clock error correction,
       * which will be performed to adjust clock in its maximum error between +/- 1ms.
       */
      float_t delta_ms(range_residual_mean_ms(gps.clock_index, info));
      if((delta_ms >= 0.9) || (delta_ms <= -0.9)){ // 0.9 ms
        std::cerr << "Detect receiver clock jump: " << delta_ms << " [ms] => ";
        float_t clock_error_shift(
            space_node_t::light_speed * 1E-3 * std::floor(delta_ms + 0.5));
        info = generator(*this, gps, clock_error_shift);
        delta_ms = range_residual_mean_ms(gps.clock_index, info);
        if((delta_ms < 0.9) && (delta_ms > -0.9)){
          std::cerr << "Fixed." << std::endl;
          // correct internal clock error
          BaseFINS::m_clock_error[gps.clock_index] += clock_error_shift;
        }else{
          std::cerr << "Skipped." << std::endl;
          return; // unknown error!
        }
      }

      BaseFINS::correct_primitive(info);
    }

  public:
    /**
     * Measurement update with GPS raw measurement
     *
     * @param gps GPS measurement
     */
    void correct(const raw_data_t &gps){
      correct_generic(gps, CorrectInfoGenerator1());
    }

    /**
     * Measurement update with GPS raw measurement and lever arm effect
     *
     * @param gps GPS measurement
     * @param lever_arm_b lever arm vector in b-frame
     * @param omega_b2i_4b angular speed vector in b-frame
     */
    void correct(const raw_data_t &gps,
        const vec3_t &lever_arm_b,
        const vec3_t &omega_b2i_4b){
      correct_generic(gps, CorrectInfoGenerator2(lever_arm_b, omega_b2i_4b));
    }
};

#endif /* __INS_GPS2_TIGHTLY_H__ */
