// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QAbstractSlider>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QSpinBox>
#include <functional>
#include <string>
#include <type_traits>

#include "Common/Config/Config.h"
#include "Common/ForEachArgument.h"
#include "Common/Invoke.h"
#include "Common/SubscriptionDispatcher.h"
#include "Core/Core.h"
#include "DolphinQt2/QtUtils/QueueOnObject.h"

template <typename T, typename = void>
struct BindablePropertyTrait;

template <typename T>
struct BindablePropertyTrait<
    T, typename std::enable_if_t<std::is_arithmetic<T>::value || std::is_enum<T>::value ||
                                 std::is_same<T, std::string>::value>>
{
  static T Get(const T& ref) { return ref; }
  static void Set(T& ref, T value)
  {
    ref = std::move(value);
    SignalDispatcher::Instance().Activate(&ref, value);
  }
  template <typename F>
  static SubscriptionHandle Subscribe(const T& ref, F&& functor)
  {
    return SignalDispatcher::Instance().Subscribe<F, T>(&ref, std::forward<F>(functor));
  }

  static void Unsubscribe(const T& ref, const SubscriptionHandle& subscription)
  {
    SignalDispatcher::Instance().Unsubscribe(subscription);
  }
};

template <typename T, typename F>
void ConnectToProperty(const T& prop, QObject* obj, F&& functor)
{
  using property_trait = BindablePropertyTrait<T>;

  functor(property_trait::Get(prop));
  auto subscription =
      property_trait::Subscribe(prop, [ obj, functor = std::forward<F>(functor) ](auto&& value) {
        QueueOnObject(
            obj, [ value = std::forward<decltype(value)>(value), functor ] { functor(value); });
      });
  QObject::connect(obj, &QObject::destroyed, [&prop, subscription = std::move(subscription) ] {
    property_trait::Unsubscribe(prop, subscription);
  });
}

template <typename T>
struct BindablePropertyTrait<Config::ConfigInfo<T>>
{
  static T Get(const Config::ConfigInfo<T>& config_info) { return Config::Get(config_info); }
  static void Set(const Config::ConfigInfo<T>& config_info, T value)
  {
    Config::SetBaseOrCurrent(config_info, std::move(value));
    SignalDispatcher::Instance().Activate(&config_info, value);
  }

  struct Subscription
  {
    // std::shared_ptr<Subscribable<>::Subscription> global_config_changed_subscription;
    SubscriptionHandle config_info_changed_subscription;
  };

  template <typename F>
  static Subscription Subscribe(const Config::ConfigInfo<T>& config_info, F&& functor)
  {
    // TODO
    // auto global_config_changed_subscription =
    // Config::OnConfigChanged().Subscribe([&config_info, functor] { functor(Get(config_info)); });
    auto config_info_changed_subscription =
        SignalDispatcher::Instance().Subscribe<F, T>(&config_info, std::forward<F>(functor));
    return {config_info_changed_subscription};
  }

  static void Unsubscribe(const Config::ConfigInfo<T>& config_info,
                          const Subscription& subscription_info)
  {
    // Config::OnConfigChanged().Unsubscribe(subscription_info.global_config_changed_subscription);
    SignalDispatcher::Instance().Unsubscribe(subscription_info.config_info_changed_subscription);
  }
};
//
// template <typename WrappedType, typename TransformedType>
// class BehaviorSubject
// {
// public:
//   using TransformFunc = std::function<TransformedType(const WrappedType&)>;
//   using Subscription = typename Subscribable<TransformedType>::Subscription;
//   using WrappedSubscription = typename Subscribable<WrappedType>::Subscription;
//
//   BehaviorSubject(WrappedType initial_value, Subscribable<WrappedType>& subscribable,
//                   TransformFunc transform_func)
//       : m_current_value(transform_func(initial_value)),
//       m_transform_func(std::move(transform_func)),
//         m_wrapped_subscribable(subscribable)
//   {
//     m_wrapped_subscription = m_wrapped_subscribable.Subscribe([=](const WrappedType& value) {
//       m_current_value = m_transform_func(value);
//       m_subscribable.Send(m_current_value);
//     });
//   }
//   BehaviorSubject(const BehaviorSubject&) = delete;
//   BehaviorSubject& operator=(const BehaviorSubject&) = delete;
//   BehaviorSubject(BehaviorSubject&&) = delete;
//   BehaviorSubject& operator=(BehaviorSubject&&) = delete;
//
//   // ~BehaviorSubject() { m_wrapped_subscribable.Unsubscribe(m_wrapped_subscription); }
//   TransformedType GetLastValue() const { return m_current_value; }
//   Subscription Subscribe(std::function<void(TransformedType)> func) const
//   {
//     return m_subscribable.Subscribe(std::move(func));
//   }
//   // void Unsubscribe(SubscriptionID id) const { m_subscribable.Unsubscribe(id); }
// private:
//   mutable TransformedType m_current_value;
//   TransformFunc m_transform_func;
//   mutable Subscribable<TransformedType> m_subscribable;
//   WrappedSubscription m_wrapped_subscription;
//   Subscribable<WrappedType>& m_wrapped_subscribable;
// };

// template <typename T, typename U>
// struct BindablePropertyTrait<BehaviorSubject<T, U>>
// {
//   using Subject = BehaviorSubject<T, U>;
//
//   static U Get(const Subject& subject) { return subject.GetLastValue(); }
//   template <typename F>
//   static std::shared_ptr<typename Subject::Subscription> Subscribe(const Subject& subject,
//                                                                    F&& functor)
//   {
//     return std::make_shared<typename Subject::Subscription>(
//         std::move(subject.Subscribe(std::forward<F>(functor))));
//   }
//   static void Unsubscribe(const Subject& subject,
//                           const std::shared_ptr<typename Subject::Subscription>& handle)
//   {
//     // TODO:
//     // subject.Unsubscribe(id);
//   }
// };

template <typename ControlClass, typename EnableIf = void>
struct BindableControlTrait;

template <>
struct BindableControlTrait<QLabel>
{
  static constexpr auto SetFunc = &QLabel::setText;
};

template <>
struct BindableControlTrait<QCheckBox>
{
  static constexpr auto SetFunc = &QCheckBox::setChecked;
  static constexpr auto SignalFunc = &QCheckBox::toggled;
};

template <>
struct BindableControlTrait<QComboBox>
{
  static constexpr auto SetFunc = &QComboBox::setCurrentIndex;  // TODO: handle out-of-range values
  static constexpr auto SignalFunc = static_cast<void (QComboBox::*)(int)>(&QComboBox::activated);
};

template <>
struct BindableControlTrait<QSpinBox>
{
  static constexpr auto SetFunc = &QSpinBox::setValue;
  static constexpr auto SignalFunc = static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged);
};

template <>
struct BindableControlTrait<QDateTimeEdit>
{
  static constexpr auto SetFunc = &QDateTimeEdit::setDateTime;
  static constexpr auto SignalFunc = &QDateTimeEdit::dateTimeChanged;
};

void ButtonGroupSetChecked(QButtonGroup* button_group, int id)
{
  button_group->button(id)->setChecked(true);
}

template <>
struct BindableControlTrait<QButtonGroup>
{
  // static constexpr auto SetFunc = &ButtonGroupSetChecked;
  static void SetFunc(QButtonGroup* button_group, int id)
  {
    button_group->button(id)->setChecked(true);
  }
  static constexpr auto SignalFunc =
      static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked);
};

template <typename T>
struct BindableControlTrait<
    T, typename std::enable_if<std::is_base_of<QAbstractSlider, T>::value>::type>
{
  static constexpr auto SetFunc = &QAbstractSlider::setValue;
  static constexpr auto SignalFunc = &QAbstractSlider::valueChanged;
};

// Set ups two-way binding between a control and an observable property. Optional function-like
// parameters transform (from property value type to control value type) and reverse_transform.
template <typename Control, typename Property>
void BindControlToProperty(Control* control, Property& prop)
{
  auto identity_transform = [](auto&& x) { return std::forward<decltype(x)>(x); };
  BindControlToProperty(control, prop, identity_transform, identity_transform);
}

template <typename Control, typename Property, typename Transform, typename ReverseTransform>
void BindControlToProperty(Control* control, Property& prop, Transform&& transform,
                           ReverseTransform&& reverse_transform)
{
  using control_trait = BindableControlTrait<Control>;
  using property_trait = BindablePropertyTrait<std::decay_t<Property>>;

  auto update_control = [ control, &prop, transform = std::forward<Transform>(transform) ]()
  {
    Invoke(control_trait::SetFunc, control, transform(property_trait::Get(prop)));
  };
  // Initially, set the view to the same value as the model
  update_control();
  // If the model changes, change the view
  ConnectToProperty(prop, control,
                    [update_control = std::move(update_control)](auto&&) { update_control(); });
  // If the view changes, update the model
  QObject::connect(control, control_trait::SignalFunc,
                   [&prop, reverse_transform = std::forward<ReverseTransform>(reverse_transform) ](
                       auto&& value) { property_trait::Set(prop, reverse_transform(value)); });
}

// Binds a control's "enabled" value to one or more observable properties. Function-like parameter
// transform (from properties value types to bool) is required when binding to more than one
// property.
template <typename Property>
void BindControlEnabledToProperty(QWidget* control, const Property& prop)
{
  auto identity_transform = [](auto&& x) { return std::forward<decltype(x)>(x); };
  BindControlEnabledToProperty(control, std::move(identity_transform), prop);
}

template <typename Transform, typename... Properties,
          typename std::enable_if_t<0 < sizeof...(Properties), int> = 0>
static void BindControlEnabledToProperty(QWidget* control, Transform&& transform,
                                         const Properties&... props)
{
  BindControlBooleanToProperty(control, &QWidget::setEnabled, transform, props...);
}

// Binds a control's "bold" value to one or more observable properties. Function-like parameter
// transform (from properties value types to bool) is required when binding to more than one
// property.
template <typename Property>
void BindControlBoldToProperty(QWidget* control, const Property& prop)
{
  auto identity_transform = [](auto&& x) { return std::forward<decltype(x)>(x); };
  BindControlBoldToProperty(control, std::move(identity_transform), prop);
}

template <typename Transform, typename... Properties,
          typename std::enable_if_t<0 < sizeof...(Properties), int> = 0>
static void BindControlBoldToProperty(QWidget* control, Transform&& transform,
                                      const Properties&... props)
{
  BindControlBooleanToProperty(control,
                               [](QWidget* widget, bool bold) {
                                 QFont bf = widget->font();
                                 bf.setBold(true);
                                 widget->setFont(bf);
                               },
                               transform, props...);
}

template <typename SetBoolean, typename Transform, typename... Properties,
          typename std::enable_if_t<0 < sizeof...(Properties), int> = 0>
static void BindControlBooleanToProperty(QWidget* control, SetBoolean&& set_boolean,
                                         Transform&& transform, const Properties&... props)
{
  auto update_control = [
    control, set_boolean = std::forward<SetBoolean>(set_boolean),
    transform = std::forward<Transform>(transform), &props...
  ]()
  {
    Invoke(set_boolean, control, transform(BindablePropertyTrait<Properties>::Get(props)...));
  };
  // Initially, set the view to the same value as the model
  update_control();
  // If the model changes, change the view
  ForEachArgument(
      [&](const auto& prop) {
        ConnectToProperty(prop, control, [update_control = std::move(update_control)](auto&&) {
          update_control();
        });
      },
      props...);
}
