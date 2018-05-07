// Copyright 2017 The Lynx Authors. All rights reserved.

#include <string>

#include "base/android/android_jni.h"
#include "base/android/convert.h"
#include "base/android/lynx_java_type.h"
#include "base/debug/memory_debug.h"
#include "base/size.h"
#include "render/label_measurer.h"
#include "render/render_object.h"
#include "runtime/platform_value.h"

#include "LabelMeasurer_jni.h"

namespace lynx {

base::Size LabelMeasurer::MeasureLabelSize(RenderObject* render_object,
                                           const base::Size& size,
                                           const std::string& text) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedLocalJavaRef<jobject> style_obj =
      base::Convert::StyleConvert(render_object->css_style());

  int width = base::Size::Descriptor::GetSize(size.width_);
  int height = base::Size::Descriptor::GetSize(size.height_);
  int widthMode = base::Size::Descriptor::GetMode(size.width_);
  int heightMode = base::Size::Descriptor::GetMode(size.height_);

  base::android::ScopedLocalJavaRef<jobject> size_obj(
      Java_LabelMeasurer_measureLabelSize(
          env,
          (jstring)base::android::LxJType::NewString(env, text.c_str()).Get(),
          style_obj.Get(), width, widthMode, height, heightMode));

  base::Size measured_size = base::Convert::SizeConvert(size_obj.Get());
  base::android::CheckException(env);
  return measured_size;
}

base::Size LabelMeasurer::MeasureLabelSizeAndSetTextLayout(
    RenderObject* render_object,
    const base::Size& size,
    const std::string& text) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::Size measured_size = MeasureLabelSize(render_object, size, text);

  auto data =
      jscore::LynxValue::MakePlatformValue(lynx_new jscore::PlatformValue(
          env, Java_LabelMeasurer_getTextLayout(env).Get()));
  render_object->SetData(RenderObject::TEXT_LAYOUT, data);

  base::android::CheckException(env);
  return measured_size;
}

base::Size LabelMeasurer::MeasureSpanSizeAndSetTextLayout(
    RenderObject *render_object, const base::Size &size,
    const std::vector<std::string> &inline_texts,
    const std::vector<CSSStyle> &inline_styles) {
  JNIEnv *env = base::android::AttachCurrentThread();

  base::android::ScopedLocalJavaRef<jobject> style_obj =
      base::Convert::StyleConvert(render_object->css_style());

  const int len = inline_styles.size();
  auto texts = base::android::JType::NewStringArray(env, len);
  auto styles = base::android::JType::NewObjectArray(env, len);
  auto text_iterator = inline_texts.begin();
  auto style_iterator = inline_styles.begin();

  for (int i = 0; i < len; i++) {
    env->SetObjectArrayElement(
        texts.Get(), i,
        base::android::JType::NewString(env, text_iterator.base()->c_str())
            .Get());
    env->SetObjectArrayElement(
        styles.Get(), i,
        base::Convert::StyleConvert(*style_iterator.base()).Get());
    text_iterator++;
    style_iterator++;
  }
  base::android::ScopedLocalJavaRef<jobject> size_obj(
      Java_LabelMeasurer_measureSpanLabelSize(
          env, texts.Get(), styles.Get(),
          style_obj.Get(),
          base::Size::Descriptor::GetSize(size.width_), 
          base::Size::Descriptor::GetMode(size.width_),
          base::Size::Descriptor::GetSize(size.height_),
          base::Size::Descriptor::GetMode(size.height_)
           ));

  base::Size measured_size = base::Convert::SizeConvert(size_obj.Get());

  auto data =
      jscore::LynxValue::MakePlatformValue(lynx_new jscore::PlatformValue(
          env, Java_LabelMeasurer_getTextLayout(env).Get()));
  render_object->SetData(RenderObject::TEXT_LAYOUT, data);

  base::android::CheckException(env);
  return measured_size;
}

bool LabelMeasurer::RegisterJNIUtils(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace lynx
