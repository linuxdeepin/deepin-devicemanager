// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "LogConfigread.h"
#include "DDLog.h"
#include "dtkcore_global.h"
#include "qglobal.h"
#include <QLoggingCategory>
#include <QObject>
#include <DConfig>

DCORE_USE_NAMESPACE
using namespace DDLog;

MLogger::MLogger(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr) {
  QByteArray logRules = qgetenv("QT_LOGGING_RULES");
  // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效
  qunsetenv("QT_LOGGING_RULES");
  // set env
  m_rules = logRules;
  // set dconfig
  m_config = DConfig::create("org.deepin.devicemanager", "org.deepin.devicemanager");
  logRules = m_config->value("rules").toByteArray();
  appendRules(logRules);
  setRules(m_rules);
  // watch dconfig
  connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
    qCDebug(appLog) << "MLogger dconfig value changed, key:" << key;
    if (key == "rules") {
      qCDebug(appLog) << "MLogger dconfig value changed, set rules";
      setRules(m_config->value(key).toByteArray());
    }
  });
}

MLogger::~MLogger() {
    qCDebug(appLog) << "MLogger destructor";
    m_config->deleteLater();
}

void MLogger::setRules(const QString &rules) {
  qCDebug(appLog) << "MLogger::setRules, rules:" << rules;
  auto tmpRules = rules;
  m_rules = tmpRules.replace(";", "\n");
  QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::appendRules(const QString &rules) {
  qCDebug(appLog) << "MLogger::appendRules, rules:" << rules;
  QString tmpRules = rules;
  tmpRules = tmpRules.replace(";", "\n");
  auto tmplist = tmpRules.split('\n');
  for (int i = 0; i < tmplist.count(); i++)
    if (m_rules.contains(tmplist.at(i))) {
      qCDebug(appLog) << "MLogger::appendRules, rules already contains:" << tmplist.at(i);
      tmplist.removeAt(i);
      i--;
    }
  if (tmplist.isEmpty()) {
    qCDebug(appLog) << "MLogger::appendRules, no new rules";
    return;
  }
  m_rules.isEmpty() ? m_rules = tmplist.join("\n")
                    : m_rules += "\n" + tmplist.join("\n");
  qCDebug(appLog) << "MLogger::appendRules, new rules:" << m_rules;
}

