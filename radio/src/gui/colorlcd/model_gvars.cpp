/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "model_gvars.h"

#include "libopenui.h"
#include "list_line_button.h"
#include "numberedit.h"
#include "opentx.h"
#include "page.h"
#include "themes/etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define GVAR_NAME_SIZE 44
#define GVAR_VAL_H (PAGE_LINE_HEIGHT * 2 - 6)
#if LCD_W > LCD_H
#define BTN_H 38
#define GVAR_VAL_W 45
#define GVAR_COLS MAX_FLIGHT_MODES
#define GVAR_H GVAR_VAL_H
#else
#define BTN_H 72
#define GVAR_VAL_W 50
#define GVAR_COLS 5
#define GVAR_H (GVAR_VAL_H * 2)
#endif

#define ETX_STATE_VALUE_SMALL_FONT LV_STATE_USER_1

void getFMExtName(char* dest, int8_t idx)
{
  getFlightModeString(dest, idx);

  FlightModeData* fmData = &g_model.flightModeData[idx - 1];
  int userNameLen = zlen(fmData->name, LEN_FLIGHT_MODE_NAME);

  if (userNameLen > 0) {
    char* s = strAppend(dest + strlen(dest), ":", 1);
    strAppend(s, fmData->name, LEN_FLIGHT_MODE_NAME);
  }
}

class GVarStyle
{
 public:
  GVarStyle() {}

  void init()
  {
    if (!styleInitDone) {
      styleInitDone = true;

      lv_style_init(&gvLabelStyle);
      lv_style_init(&gvNameStyle);
    }

    // Update in case FM state changed
    lv_style_set_width(&gvNameStyle,
                       modelFMEnabled() ? GVAR_NAME_SIZE : GVAR_NAME_SIZE * 2);
    lv_style_set_height(&gvLabelStyle, modelFMEnabled() ? PAGE_LINE_HEIGHT - 6
                                                        : PAGE_LINE_HEIGHT);
  }

  lv_obj_t* newFMCont(lv_obj_t* parent, uint8_t flightMode);
  lv_obj_t* newName(lv_obj_t* parent, const char* name);
  lv_obj_t* newLabel(lv_obj_t* parent, const char* label);

  lv_style_t gvNameStyle;
  lv_style_t gvLabelStyle;

 private:
  bool styleInitDone;
};

static GVarStyle gvarStyle;

static void gv_group_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->pad_zero, LV_PART_MAIN);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
}

static const lv_obj_class_t gv_group_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = gv_group_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVAR_VAL_W * GVAR_COLS,
    .height_def = GVAR_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_obj_t),
};

static void gv_fmcont_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->pad_zero, LV_PART_MAIN);
  etx_std_ctrl_colors(obj, LV_PART_MAIN);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
}

static const lv_obj_class_t gv_fmcont_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = gv_fmcont_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVAR_VAL_W,
    .height_def = GVAR_VAL_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_obj_t),
};

lv_obj_t* GVarStyle::newFMCont(lv_obj_t* parent, uint8_t flightMode)
{
  auto obj = etx_create(&gv_fmcont_class, parent);
  lv_obj_set_pos(obj, (flightMode % GVAR_COLS) * GVAR_VAL_W,
                 (flightMode / GVAR_COLS) * GVAR_VAL_H);

  return obj;
}

static void gv_name_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->pad_left_2, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
  etx_obj_add_style(obj, gvarStyle.gvNameStyle, LV_PART_MAIN);
}

static const lv_obj_class_t gv_name_class = {
    .base_class = &lv_label_class,
    .constructor_cb = gv_name_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVAR_NAME_SIZE,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* GVarStyle::newName(lv_obj_t* parent, const char* name)
{
  auto obj = etx_create(&gv_name_class, parent);
  lv_label_set_text(obj, name);

  return obj;
}

static void gv_label_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX);
  etx_obj_add_style(obj, gvarStyle.gvLabelStyle, LV_PART_MAIN);
  lv_obj_set_pos(obj, 0, 0);
}

static const lv_obj_class_t gv_label_class = {
    .base_class = &lv_label_class,
    .constructor_cb = gv_label_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVAR_VAL_W,
    .height_def = PAGE_LINE_HEIGHT - 6,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* GVarStyle::newLabel(lv_obj_t* parent, const char* label)
{
  auto obj = etx_create(&gv_label_class, parent);
  lv_label_set_text(obj, label);

  return obj;
}

static void gv_value_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX, LV_PART_MAIN | ETX_STATE_VALUE_SMALL_FONT);
  lv_obj_set_pos(obj, 0, PAGE_LINE_HEIGHT - 6);
}

static const lv_obj_class_t gv_value_class = {
    .base_class = &lv_label_class,
    .constructor_cb = gv_value_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVAR_VAL_W,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

class GVarButton : public ListLineButton
{
 public:
  GVarButton(Window* parent, const rect_t& rect, uint8_t gvar) :
      ListLineButton(parent, gvar)
  {
    padAll(PAD_ZERO);
    padColumn(PAD_MEDIUM);
    setHeight(BTN_H);
    lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        lvobj, !modelFMEnabled() ? LV_FLEX_ALIGN_CENTER : LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
    if (!modelFMEnabled()) padLeft(8);

    lv_obj_add_event_cb(lvobj, GVarButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (GVarButton*)lv_obj_get_user_data(target);
    if (line) line->build(e);
  }

 protected:
  bool init = false;
  uint8_t currentFlightMode = 0;  // used for invalidation
  lv_obj_t* fmCont[MAX_FLIGHT_MODES];
  lv_obj_t* valueTexts[MAX_FLIGHT_MODES];
  gvar_t values[MAX_FLIGHT_MODES];

  int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    if (init) {
      if (modelFMEnabled()) {
        uint8_t newFM = getFlightMode();
        if (currentFlightMode != newFM) {
          lv_obj_add_state(fmCont[newFM], LV_STATE_CHECKED);
          lv_obj_clear_state(fmCont[currentFlightMode], LV_STATE_CHECKED);

          currentFlightMode = newFM;
        }
      }

      for (int flightMode = 0; flightMode < numFlightModes(); flightMode++) {
        FlightModeData* fmData = &g_model.flightModeData[flightMode];
        if (values[flightMode] != fmData->gvars[index]) {
          updateValueText(flightMode);
        }
      }
    }
  }

  void build(lv_event_t* e)
  {
    if (init) return;

    currentFlightMode = getFlightMode();

    gvarStyle.newName(lvobj, getGVarString(index));

    if (modelFMEnabled()) {
      auto container = etx_create(&gv_group_class, lvobj);

      for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
        fmCont[flightMode] = gvarStyle.newFMCont(container, flightMode);
        if (flightMode == currentFlightMode) {
          lv_obj_add_state(fmCont[flightMode], LV_STATE_CHECKED);
        }

        char label[16] = {};
        getFlightModeString(label, flightMode + 1);
        gvarStyle.newLabel(fmCont[flightMode], label);

        valueTexts[flightMode] =
            etx_create(&gv_value_class, fmCont[flightMode]);

        updateValueText(flightMode);
      }
    } else {
      valueTexts[0] = lv_label_create(lvobj);

      updateValueText(0);
    }

    init =true;

    lv_obj_update_layout(lvobj);

    if (e) {
      auto param = lv_event_get_param(e);
      lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
    }
  }

  void updateValueText(uint8_t flightMode)
  {
    lv_obj_t* field = valueTexts[flightMode];
    gvar_t value = g_model.flightModeData[flightMode].gvars[index];
    values[flightMode] = value;

    if (value > GVAR_MAX) {
      uint8_t fm = value - GVAR_MAX - 1;
      if (fm >= flightMode) fm += 1;
      char label[16] = {};
      getFlightModeString(label, fm + 1);

      lv_label_set_text(field, label);
    } else {
      uint8_t unit = g_model.gvars[index].unit;
      const char* suffix = (unit == 1) ? "%" : "";
      uint8_t prec = g_model.gvars[index].prec;
      if (prec)
        lv_label_set_text_fmt(field, "%d.%01u%s", value / 10,
                              (value < 0) ? (-value) % 10 : value % 10, suffix);
      else
        lv_label_set_text_fmt(field, "%d%s", value, suffix);
      if (unit) {
        if (value <= -1000 || value >= 1000 || (prec && (value <= -100))) {
          lv_obj_add_state(field, ETX_STATE_VALUE_SMALL_FONT);
        } else {
          lv_obj_clear_state(field, ETX_STATE_VALUE_SMALL_FONT);
        }
      }
    }
  }

  bool isActive() const override { return false; }
  void refresh() override {}
};

class GVarEditWindow : public Page
{
 public:
  explicit GVarEditWindow(uint8_t gvarIndex) :
      Page(ICON_MODEL_GVARS), index(gvarIndex)
  {
    buildHeader(header);
    buildBody(body);
  }

 protected:
  uint8_t index;
  gvar_t lastGVar = 0;
  bool refreshTitle = true;
  uint8_t lastFlightMode = 255;  // Force initial setting of header title
  NumberEdit* min = nullptr;
  NumberEdit* max = nullptr;
  NumberEdit* values[MAX_FLIGHT_MODES] = {};
  StaticText* gVarInHeader = nullptr;

  int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

  void buildHeader(Window* window)
  {
    header->setTitle(STR_MENU_GLOBAL_VARS);
    gVarInHeader = header->setTitle2("");
  }

  void checkEvents()
  {
    Page::checkEvents();

    auto curFM = getFlightMode();
    auto fmData = &g_model.flightModeData[curFM];

    if (gVarInHeader && ((lastFlightMode != curFM) ||
                         (lastGVar != fmData->gvars[index]) || refreshTitle)) {
      char label[32];
      refreshTitle = false;
      lastFlightMode = curFM;
      lastGVar = fmData->gvars[index];
      sprintf(label, "%s%d=", STR_GV, index + 1);
      if (lastGVar > GVAR_MAX) {
        uint8_t fm = lastGVar - GVAR_MAX - 1;
        if (fm >= curFM) fm++;
        getFMExtName(label + strlen(label), fm + 1);
      } else {
        strcat(label, getGVarValue(index, lastGVar, 0).c_str());
      }
      gVarInHeader->setText(label);
    }
  }

  void setProperties(int onlyForFlightMode = -1)
  {
    GVarData* gvar = &g_model.gvars[index];
    int32_t minValue = GVAR_MIN + gvar->min;
    int32_t maxValue = GVAR_MAX - gvar->max;
    const char* suffix = gvar->unit ? "%" : "";

    if (min && max) {
      min->setMax(maxValue);
      max->setMin(minValue);

      min->setSuffix(suffix);
      max->setSuffix(suffix);

      if (gvar->prec) {
        min->setTextFlag(PREC1);
        max->setTextFlag(PREC1);
      } else {
        min->clearTextFlag(PREC1);
        max->clearTextFlag(PREC1);
      }

      min->update();
      max->update();
    }
    FlightModeData* fmData;
    for (int fm = 0; fm < numFlightModes(); fm++) {
      if (values[fm] == nullptr)  // KLK: the order of calls has changed and
                                  // this might not be initialized yet.
        continue;

      if (onlyForFlightMode >= 0 && fm != onlyForFlightMode) continue;
      fmData = &g_model.flightModeData[fm];

      // custom value
      if (fmData->gvars[index] <= GVAR_MAX || fm == 0) {
        values[fm]->setMin(GVAR_MIN + gvar->min);
        values[fm]->setMax(GVAR_MAX - gvar->max);
        // Update value if outside min/max range
        values[fm]->setValue(values[fm]->getValue());

        if (gvar->prec)
          values[fm]->setTextFlag(PREC1);
        else
          values[fm]->clearTextFlag(PREC1);

        values[fm]->setDisplayHandler(nullptr);
      } else {
        values[fm]->setMin(GVAR_MAX + 1);
        values[fm]->setMax(GVAR_MAX + MAX_FLIGHT_MODES - 1);
        values[fm]->setDisplayHandler([=](int32_t value) {
          uint8_t targetFlightMode = value - GVAR_MAX - 1;
          if (targetFlightMode >= fm) targetFlightMode++;
          char label[16];
          getFlightModeString(label, targetFlightMode + 1);
          return std::string(label);
        });
      }

      values[fm]->setSuffix(suffix);
    }
  }

  void buildBody(Window* window)
  {
    static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};

    window->setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

    auto line = window->newLine(grid);

    GVarData* gvar = &g_model.gvars[index];

    new StaticText(line, rect_t{}, STR_NAME);
    grid.nextCell();
    new ModelTextEdit(line, rect_t{}, gvar->name, LEN_GVAR_NAME);

    line = window->newLine(grid);

    static const char* const strUnits[] = { "-", "%" };
    new StaticText(line, rect_t{}, STR_UNIT);
    grid.nextCell();
    new Choice(line, rect_t{}, strUnits, 0, 1, GET_DEFAULT(gvar->unit),
               [=](int16_t newValue) {
                 refreshTitle = (gvar->unit != newValue);
                 gvar->unit = newValue;
                 SET_DIRTY();
                 setProperties();
               });

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_PRECISION);
    grid.nextCell();
    new Choice(line, rect_t{}, STR_VPREC, 0, 1, GET_DEFAULT(gvar->prec),
               [=](int16_t newValue) {
                 refreshTitle = (gvar->prec != newValue);
                 gvar->prec = newValue;
                 SET_DIRTY();
                 setProperties();
               });

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_MIN);
    grid.nextCell();
    min = new NumberEdit(
        line, rect_t{}, GVAR_MIN, GVAR_MAX - gvar->max,
        [=] { return gvar->min + GVAR_MIN; },
        [=](int32_t newValue) {
          gvar->min = newValue - GVAR_MIN;
          SET_DIRTY();
          setProperties();
        });
    min->setAccelFactor(16);

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_MAX);
    grid.nextCell();
    max = new NumberEdit(
        line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX,
        [=] { return GVAR_MAX - gvar->max; },
        [=](int32_t newValue) {
          gvar->max = GVAR_MAX - newValue;
          SET_DIRTY();
          setProperties();
        });
    max->setAccelFactor(16);

    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_POPUP);
    grid.nextCell();
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(gvar->popup));

    line = window->newLine(grid);
    char flightModeName[16];
    FlightModeData* fmData;

    for (int flightMode = 0; flightMode < numFlightModes(); flightMode++) {
      fmData = &g_model.flightModeData[flightMode];

      if (modelFMEnabled()) {
        getFMExtName(flightModeName, flightMode + 1);
        new StaticText(line, rect_t{}, flightModeName);
      } else {
        new StaticText(line, rect_t{}, STR_VALUE);
      }

      if (flightMode > 0) {
        auto cb = new ToggleSwitch(
            line, rect_t{}, [=] { return fmData->gvars[index] <= GVAR_MAX; },
            [=](uint8_t checked) {
              fmData->gvars[index] = checked ? 0 : GVAR_MAX + 1;
              setProperties(flightMode);
            });
        lv_obj_set_style_grid_cell_x_align(cb->getLvObj(), LV_GRID_ALIGN_END,
                                           0);
        lv_obj_invalidate(cb->getLvObj());
      } else {
        grid.nextCell();
      }

      values[flightMode] = new NumberEdit(
          line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX + MAX_FLIGHT_MODES - 1,
          GET_SET_DEFAULT(fmData->gvars[index]));
      values[flightMode]->setAccelFactor(16);
      line = window->newLine(grid);
    }

    setProperties();
    lv_obj_set_height(window->getLvObj(),
                      LCD_H - lv_obj_get_height(header->getLvObj()));
    lv_obj_set_height(lvobj, LCD_H);
  }
};

ModelGVarsPage::ModelGVarsPage() :
    PageTab(STR_MENU_GLOBAL_VARS, ICON_MODEL_GVARS)
{
  gvarStyle.init();
}

void ModelGVarsPage::rebuild(Window* window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());
  window->clear();
  build(window);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelGVarsPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  for (uint8_t index = 0; index < MAX_GVARS; index++) {
    auto button = new GVarButton(window, rect_t{}, index);
    button->setPressHandler([=]() {
      Menu* menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() {
        Window* editWindow = new GVarEditWindow(index);
        editWindow->setCloseHandler([=]() { rebuild(window); });
      });
      menu->addLine(STR_CLEAR, [=]() {
        for (auto& flightMode : g_model.flightModeData) {
          flightMode.gvars[index] = 0;
        }
        storageDirty(EE_MODEL);
      });
      return 0;
    });
  }
}
