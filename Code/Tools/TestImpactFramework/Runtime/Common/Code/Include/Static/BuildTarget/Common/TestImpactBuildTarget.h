/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Target/Common/TestImpactTarget.h>

#include <AzCore/std/containers/variant.h>
#include <AzCore/std/functional.h>

namespace TestImpact
{
    //! Enumeration to facilitate runtime determination of build target types. 
    enum class BuildTargetType : AZ::u8
    {
        TestTarget,
        ProductionTarget
    };

    //! Common wrapper for repository build targets, be they production targets or test targets.
    template<typename ProductionTarget, typename TestTarget>
    class BuildTarget
    {
    public:
        //! Constructor for test targets.
        BuildTarget(const TestTarget* testTarget);

        //! Constructor for production targets.
        BuildTarget(const ProductionTarget* productionTarget);
    
        //! Returns the generic target pointer for this parent, otherwise nullptr.
        const Target* GetTarget() const;
    
        //! Returns the test target pointer for this parent (if any), otherwise nullptr.
        const TestTarget* GetTestTarget() const;
    
        //! Returns the production target pointer for this parent (if any), otherwise nullptr.
        const ProductionTarget* GetProductionTarget() const;

        //! Returns the target type at runtime.
        BuildTargetType GetTargetType() const;
    
        //! Visits the target type at compile time.
        template<typename Visitor>
        void Visit(const Visitor& visitor) const;

    private:
        //! Compile time check for whether or not this build target is a production target.
        template<typename Target>
        static constexpr bool IsProductionTarget =
            AZStd::is_same_v<ProductionTarget, AZStd::remove_const_t<AZStd::remove_pointer_t<AZStd::decay_t<Target>>>>;

        //! Compile time check for whether or not this build target is a test target.
        template<typename Target>
        static constexpr bool IsTestTarget =
            AZStd::is_same_v<TestTarget, AZStd::remove_const_t<AZStd::remove_pointer_t<AZStd::decay_t<Target>>>>;
        AZStd::variant<const TestTarget*, const ProductionTarget*> m_target;

        //! The build target type (either production or test)/.
        BuildTargetType m_type;
    };

    template<typename ProductionTarget, typename TestTarget>
    BuildTarget<ProductionTarget, TestTarget>::BuildTarget(const TestTarget* testTarget)
        : m_target(testTarget)
        , m_type(BuildTargetType::TestTarget)
    {
    }

    template<typename ProductionTarget, typename TestTarget>
    BuildTarget<ProductionTarget, TestTarget>::BuildTarget(const ProductionTarget* productionTarget)
        : m_target(productionTarget)
        , m_type(BuildTargetType::ProductionTarget)
    {
    }

    template<typename ProductionTarget, typename TestTarget>
    const Target* BuildTarget<ProductionTarget, TestTarget>::GetTarget() const
    {
        const Target* returnTarget = nullptr;
        Visit(
            [&returnTarget](auto&& target)
            {
                returnTarget = &(*target);
            });

        return returnTarget;
    }

    template<typename ProductionTarget, typename TestTarget>
    const TestTarget* BuildTarget<ProductionTarget, TestTarget>::GetTestTarget() const
    {
        const TestTarget* testTarget = nullptr;
        Visit(
            [&testTarget](auto&& target)
            {
                if constexpr (IsTestTarget<decltype(target)>)
                {
                    testTarget = target;
                }
            });

        return testTarget;
    }

    template<typename ProductionTarget, typename TestTarget>
    const ProductionTarget* BuildTarget<ProductionTarget, TestTarget>::GetProductionTarget() const
    {
        const ProductionTarget* productionTarget = nullptr;
        Visit(
            [&productionTarget](auto&& target)
            {
                if constexpr (IsProductionTarget<decltype(target)>)
                {
                    productionTarget = target;
                }
            });

        return productionTarget;
    }

    template<typename ProductionTarget, typename TestTarget>
    BuildTargetType BuildTarget<ProductionTarget, TestTarget>::GetTargetType() const
    {       
        return m_type;
    }

    template<typename ProductionTarget, typename TestTarget>
    template<typename Visitor>
    void BuildTarget<ProductionTarget, TestTarget>::Visit(const Visitor& visitor) const
    {
        AZStd::visit(visitor, m_target);
    }

    //! Comparison between build target types.
    template<typename ProductionTarget, typename TestTarget>
    bool operator==(const BuildTarget<ProductionTarget, TestTarget>& lhs, const BuildTarget<ProductionTarget, TestTarget>& rhs)
    {
        return lhs.GetTarget() == rhs.GetTarget();
    }

    //! Optional for build target types.
    template<typename ProductionTarget, typename TestTarget>
    using OptionalBuildTarget = AZStd::optional<BuildTarget<ProductionTarget, TestTarget>>;
} // namespace TestImpact

namespace AZStd
{
    //! Hash function for BuildTarget types for use in maps and sets.
    template<typename ProductionTarget, typename TestTarget>
    struct hash<TestImpact::BuildTarget<ProductionTarget, TestTarget>>
    {
        size_t operator()(const TestImpact::BuildTarget<ProductionTarget, TestTarget>& buildTarget) const noexcept
        {
            return reinterpret_cast<size_t>(buildTarget.GetTarget());
        }
    };
} // namespace AZStd
