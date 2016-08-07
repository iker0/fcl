/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef FCL_SHAPE_HALFSPACE_H
#define FCL_SHAPE_HALFSPACE_H

#include "fcl/shape/shape_base.h"
#include "fcl/shape/compute_bv.h"
#include "fcl/BV/OBB.h"
#include "fcl/BV/RSS.h"
#include "fcl/BV/OBBRSS.h"
#include "fcl/BV/kDOP.h"
#include "fcl/BV/kIOS.h"

namespace fcl
{

/// @brief Half Space: this is equivalent to the Planed in ODE. The separation plane is defined as n * x = d;
/// Points in the negative side of the separation plane (i.e. {x | n * x < d}) are inside the half space and points
/// in the positive side of the separation plane (i.e. {x | n * x > d}) are outside the half space
template <typename ScalarT>
class Halfspace : public ShapeBase<ScalarT>
{
public:

  using Scalar = ScalarT;

  /// @brief Construct a half space with normal direction and offset
  Halfspace(const Vector3<ScalarT>& n, ScalarT d);

  /// @brief Construct a plane with normal direction and offset
  Halfspace(ScalarT a, ScalarT b, ScalarT c, ScalarT d_);

  Halfspace();

  ScalarT signedDistance(const Vector3<ScalarT>& p) const;

  ScalarT distance(const Vector3<ScalarT>& p) const;

  /// @brief Compute AABB
  void computeLocalAABB() override;

  /// @brief Get node type: a half space
  NODE_TYPE getNodeType() const override;
  
  /// @brief Planed normal
  Vector3<ScalarT> n;
  
  /// @brief Planed offset
  ScalarT d;

protected:

  /// @brief Turn non-unit normal into unit
  void unitNormalTest();
};

using Halfspacef = Halfspace<float>;
using Halfspaced = Halfspace<double>;

template <typename ScalarT>
Halfspace<ScalarT> transform(
    const Halfspace<ScalarT>& a, const Transform3<ScalarT>& tf)
{
  /// suppose the initial halfspace is n * x <= d
  /// after transform (R, T), x --> x' = R x + T
  /// and the new half space becomes n' * x' <= d'
  /// where n' = R * n
  ///   and d' = d + n' * T

  Vector3<ScalarT> n = tf.linear() * a.n;
  ScalarT d = a.d + n.dot(tf.translation());

  return Halfspace<ScalarT>(n, d);
}

//============================================================================//
//                                                                            //
//                              Implementations                               //
//                                                                            //
//============================================================================//

//==============================================================================
template <typename ScalarT>
Halfspace<ScalarT>::Halfspace(const Vector3<ScalarT>& n, ScalarT d)
  : ShapeBase<ScalarT>(), n(n), d(d)
{
  unitNormalTest();
}

//==============================================================================
template <typename ScalarT>
Halfspace<ScalarT>::Halfspace(ScalarT a, ScalarT b, ScalarT c, ScalarT d)
  : ShapeBase<ScalarT>(), n(a, b, c), d(d)
{
  unitNormalTest();
}

//==============================================================================
template <typename ScalarT>
Halfspace<ScalarT>::Halfspace() : ShapeBase<ScalarT>(), n(1, 0, 0), d(0)
{
  // Do nothing
}

//==============================================================================
template <typename ScalarT>
ScalarT Halfspace<ScalarT>::signedDistance(const Vector3<ScalarT>& p) const
{
  return n.dot(p) - d;
}

//==============================================================================
template <typename ScalarT>
ScalarT Halfspace<ScalarT>::distance(const Vector3<ScalarT>& p) const
{
  return std::abs(n.dot(p) - d);
}

//==============================================================================
template <typename ScalarT>
void Halfspace<ScalarT>::computeLocalAABB()
{
  computeBV(*this, Transform3<ScalarT>::Identity(), this->aabb_local);
  this->aabb_center = this->aabb_local.center();
  this->aabb_radius = (this->aabb_local.min_ - this->aabb_center).norm();
}

//==============================================================================
template <typename ScalarT>
NODE_TYPE Halfspace<ScalarT>::getNodeType() const
{
  return GEOM_HALFSPACE;
}

//==============================================================================
template <typename ScalarT>
void Halfspace<ScalarT>::unitNormalTest()
{
  ScalarT l = n.norm();
  if(l > 0)
  {
    ScalarT inv_l = 1.0 / l;
    n *= inv_l;
    d *= inv_l;
  }
  else
  {
    n << 1, 0, 0;
    d = 0;
  }
}

} // namespace fcl

#include "fcl/shape/detail/bv_computer_halfspace.h"

#endif