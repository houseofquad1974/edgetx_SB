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

#include "model_templates.h"

#include "standalone_lua.h"
#include "str_functions.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

TemplatePage::TemplatePage() : Page(ICON_MODEL_SELECT)
{
  body.padAll(0);

  auto form = new FormWindow(&body, rect_t{});
  form->setFlexLayout();
  form->padAll(4);

  FlexGridLayout grid(col_dsc, row_dsc, 4);

  auto line = form->newLine(&grid);

  listWindow = new FormWindow(line, rect_t{});
  listWindow->setFlexLayout(LV_FLEX_FLOW_COLUMN, 8);
  listWindow->setHeight(body.height() - 16);
  lv_obj_set_flex_align(listWindow->getLvObj(), LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_BETWEEN);
  lv_obj_set_grid_cell(listWindow->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  infoLabel = lv_label_create(line->getLvObj());
  lv_obj_set_height(infoLabel, body.height() - 16);
  lv_obj_set_style_text_align(infoLabel, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_grid_cell(infoLabel, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
}

void TemplatePage::updateInfo()
{
  if (buffer[0]) {
  VfsFile fp;
  VfsError res = VirtualFS::instance().openFile(fp, buffer, VfsOpenFlags::READ);
  size_t bytesRead = 0;
  if (res == VfsError::OK) {
    fp.read(infoText, LEN_INFO_TEXT, bytesRead);
    fp.close();
    }
    infoText[bytesRead] = '\0';

  }

  if (infoText[0] == 0) {
    lv_label_set_text(infoLabel, STR_NO_INFORMATION);
    lv_obj_set_style_text_color(infoLabel, makeLvColor(COLOR_THEME_DISABLED),
                                0);
  } else {
    lv_label_set_text(infoLabel, infoText);
    lv_obj_set_style_text_color(infoLabel, makeLvColor(COLOR_THEME_PRIMARY1),
                                0);
  }
}

#if defined(HARDWARE_KEYS)
void TemplatePage::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  } else {
    Page::onEvent(event);
  }
}
#endif

class SelectTemplate : public TemplatePage
{
 public:
  SelectTemplate(SelectTemplateFolder* tp, std::string folder) : templateFolderPage(tp)
  {
    header.setTitle(STR_MANAGE_MODELS);
    header.setTitle2(STR_NEW_MODEL);

    char path[LEN_PATH + 1];
    snprintf(path, LEN_PATH, "%s/%s", TEMPLATES_PATH, folder.c_str());

    std::list<std::string> files;
    FILINFO fno;
    DIR dir;
    FRESULT res = f_opendir(&dir, path);

    Button* firstButton = nullptr;
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  std::list<std::string> files;
  VirtualFS& vfs = VirtualFS::instance();
  VfsFileInfo fno;
  VfsDir dir;
  VfsError res = vfs.openDirectory(dir, path);

  if (res == VfsError::OK) {
    // read all entries
    for (;;) {
      res = dir.read(fno);
      if (res != VfsError::OK || fno.getName()[0] == 0)
        break; // Break on error or end of dir
      const char* fName = fno.getName();
      if (strlen(fName) > STORAGE_SCREEN_FILE_LENGTH)
        continue;
      if (fName[0] == '.')
        continue;
      if (fno.getType() == VfsType::FILE) {
        const char *ext = vfs.getFileExtension(fName);
        if(ext && !strcasecmp(ext, YAML_EXT)) {
          int len = ext - fName;
          if (len < FF_MAX_LFN) {
            char name[FF_MAX_LFN] = { 0 };
            strncpy(name, fName, len);
            files.push_back(name);
          }
        }
      }

      files.sort(compare_nocase);

      for (auto name : files) {
        auto tb = new TextButton(
            listWindow, rect_t{0, 0, lv_pct(100), PAGE_LINE_HEIGHT * 2}, name,
            [=]() -> uint8_t {
              deleteLater();
              templateFolderPage->doUpdate(folder, name);
              return 0;
            });
        tb->setFocusHandler([=](bool active) {
          if (active) {
            snprintf(buffer, LEN_BUFFER, "%s/%s%s", path, name.c_str(),
                     TEXT_EXT);
            updateInfo();
          }
        });

        if (!firstButton) firstButton = tb;
      }
    }

  dir.close();

    if (files.size() == 0) {
      new StaticText(listWindow, rect_t{0, 0, lv_pct(100), lv_pct(50)},
                     STR_NO_TEMPLATES, 0, COLOR_THEME_PRIMARY1);
    } else {
      lv_group_focus_obj(firstButton->getLvObj());
    }
  }

 protected:
  SelectTemplateFolder* templateFolderPage;
};

SelectTemplateFolder::SelectTemplateFolder(std::function<void(std::string folder, std::string)> update)
{
  this->update = update;

  header.setTitle(STR_MANAGE_MODELS);
  header.setTitle2(STR_NEW_MODEL);

  auto tfb = new TextButton(listWindow,
                            rect_t{0, 0, lv_pct(100), PAGE_LINE_HEIGHT * 2},
                            STR_BLANK_MODEL, [=]() -> uint8_t {
                              doUpdate("", "");
                              return 0;
                            });
  tfb->setFocusHandler([=](bool active) {
    if (active) {
      buffer[0] = 0;
      strcpy(infoText, STR_BLANK_MODEL_INFO);
      updateInfo();
    }
  });

  std::list<std::string> directories;
  VfsFileInfo fno;
  VfsDir dir;
  VfsError res = VirtualFS::instance().openDirectory(dir, TEMPLATES_PATH);

  if (res == VfsError::OK) {
    // read all entries
    for (;;) {
      res = dir.read(fno);
      if (res != VfsError::OK || fno.getName() == 0)
        break; // Break on error or end of dir
      const char* fName = fno.getName();
      if (strlen(fName) > STORAGE_SCREEN_FILE_LENGTH)
        continue;
      if (fName[0] == '.')
        continue;
      if (fno.getType() == VfsType::DIR)
        directories.push_back(fName);
    }

    directories.sort(compare_nocase);

    for (auto name : directories) {
#if not defined(LUA)
      // Don't show wizards dir if no lua
      if (!strcasecmp(name.c_str(), "WIZARD") == 0) {
#endif
        auto tfb = new TextButton(
            listWindow, rect_t{0, 0, lv_pct(100), PAGE_LINE_HEIGHT * 2}, name,
            [=]() -> uint8_t {
              new SelectTemplate(this, name);
              return 0;
            });
        tfb->setFocusHandler([=](bool active) {
          if (active) {
            snprintf(buffer, LEN_BUFFER, "%s/%s/about%s", TEMPLATES_PATH,
                     name.c_str(), TEXT_EXT);
            updateInfo();
          }
        });
#if not defined(LUA)
      }
#endif
    }
  }

  dir.close();

  if (directories.size() == 0) {
    new StaticText(listWindow, rect_t{0, 0, lv_pct(100), lv_pct(50)},
                   STR_NO_TEMPLATES, 0, COLOR_THEME_PRIMARY1);
  }

  lv_group_focus_obj(tfb->getLvObj());
}
