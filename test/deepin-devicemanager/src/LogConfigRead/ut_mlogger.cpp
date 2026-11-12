// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <DConfig>

#define private public
#define protected public
#include "LogConfigread.h"
#undef private
#undef protected

// ---- DConfig stub helpers -------------------------------------------------

// Pre-created DConfig used as the return value of the stubbed create().
static Dtk::Core::DConfig *ut_mlogger_config = nullptr;

// Replacement for DConfig::create(appId, name, subpath, parent).
static Dtk::Core::DConfig *ut_dconfig_create(const QString &, const QString &,
                                              const QString &, QObject *)
{
    return ut_mlogger_config;
}

// Replacement for DConfig::value(key, fallback) — returns empty so that
// appendRules receives nothing and only env-var rules end up in m_rules.
static QVariant ut_dconfig_value(const Dtk::Core::DConfig *, const QString &,
                                 const QVariant &)
{
    return QVariant(QByteArray(""));
}

// ===========================================================================
class UT_MLogger : public UT_HEAD
{
public:
    void SetUp() override
    {
        // MLogger ctor reads QT_LOGGING_RULES then unsets it.
        qputenv("QT_LOGGING_RULES", "a.debug=true");
        ut_mlogger_config = new Dtk::Core::DConfig("org.deepin.devicemanager");

        m_stub.set(
            (Dtk::Core::DConfig * (*)(const QString &, const QString &, const QString &, QObject *))
                &Dtk::Core::DConfig::create,
            ut_dconfig_create);
        m_stub.set(
            (QVariant (Dtk::Core::DConfig::*)(const QString &, const QVariant &) const)
                &Dtk::Core::DConfig::value,
            ut_dconfig_value);
    }

    void TearDown() override
    {

        delete ut_mlogger_config;
        ut_mlogger_config = nullptr;
        qunsetenv("QT_LOGGING_RULES");
    }

    Stub m_stub;
};

// ---- Constructor -----------------------------------------------------------
TEST_F(UT_MLogger, UT_MLogger_ctor_readsEnvLoggingRules)
{
    MLogger logger;
    // env "a.debug=true" has no ';' → m_rules should equal env value as-is.
    EXPECT_EQ(QString("a.debug=true"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_ctor_readsEnvLoggingRulesWithSemicolons)
{
    qputenv("QT_LOGGING_RULES", "a.debug=true;b.debug=false");
    MLogger logger;
    // ctor calls setRules internally → ';' replaced with '\n'.
    EXPECT_EQ(QString("a.debug=true\nb.debug=false"), logger.m_rules);
}

// ---- setRules --------------------------------------------------------------
TEST_F(UT_MLogger, UT_MLogger_setRules_replacesSemicolons)
{
    MLogger logger;
    logger.setRules("a.debug=true;b.debug=false;c.debug=true");
    EXPECT_EQ(QString("a.debug=true\nb.debug=false\nc.debug=true"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_setRules_noSemicolons)
{
    MLogger logger;
    logger.setRules("a.debug=true");
    EXPECT_EQ(QString("a.debug=true"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_setRules_emptyString)
{
    MLogger logger;
    logger.setRules("");
    EXPECT_EQ(QString(""), logger.m_rules);
}

// ---- appendRules -----------------------------------------------------------
TEST_F(UT_MLogger, UT_MLogger_appendRules_appendsNewRules)
{
    MLogger logger;
    logger.m_rules = "a.debug=true";
    logger.appendRules("b.debug=false");
    EXPECT_EQ(QString("a.debug=true\nb.debug=false"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_appendRules_deduplicatesExistingRules)
{
    MLogger logger;
    logger.m_rules = "a.debug=true";
    logger.appendRules("a.debug=true");
    // Duplicate rule should not be appended.
    EXPECT_EQ(QString("a.debug=true"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_appendRules_emptyRulesDoesNothing)
{
    MLogger logger;
    logger.m_rules = "a.debug=true";
    logger.appendRules("");
    // Empty input → split gives [""], which is "contained" → no append.
    EXPECT_EQ(QString("a.debug=true"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_appendRules_mixedNewAndExisting)
{
    MLogger logger;
    logger.m_rules = "a.debug=true";
    logger.appendRules("a.debug=true;b.debug=false");
    // Only the new rule "b.debug=false" should be appended.
    EXPECT_EQ(QString("a.debug=true\nb.debug=false"), logger.m_rules);
}

TEST_F(UT_MLogger, UT_MLogger_appendRules_toEmptyRules)
{
    MLogger logger;
    logger.m_rules = "";
    logger.appendRules("a.debug=true");
    // When m_rules is empty, tmplist.join is assigned directly.
    EXPECT_EQ(QString("a.debug=true"), logger.m_rules);
}

// ---- rules() getter --------------------------------------------------------
TEST_F(UT_MLogger, UT_MLogger_rules_returnsMRules)
{
    MLogger logger;
    logger.m_rules = "test.rule=true";
    EXPECT_EQ(QString("test.rule=true"), logger.rules());
}

// ---- Destructor ------------------------------------------------------------
TEST_F(UT_MLogger, UT_MLogger_destructor_noCrash)
{
    MLogger *logger = new MLogger;
    delete logger;
    SUCCEED();
}
