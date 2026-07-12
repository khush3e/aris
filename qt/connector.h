/* Connector Class to connect QML with underlying C logic.

   Copyright (C) 2023 Saksham Attri.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QVariantList>
#include <QObject>
#include <QHash>
#include "../src/typedef.h"
#include "../src/process.h"
#include "proofdata.h"
#include "goaldata.h"
#include "proofmodel.h"

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QObject *parent = nullptr);

    void reverseMapInit();

    Q_PROPERTY(QString evalText READ evalText WRITE setEvalText NOTIFY evalTextChanged)
    Q_PROPERTY(QString autoSaveStatus READ autoSaveStatus NOTIFY autoSaveStatusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

    QString evalText() const;
    void setEvalText(const QString &newEvalText);

    QString autoSaveStatus() const;
    void setAutoSaveStatus(const QString &status);

    QString lastError() const;

    void genIndices(const ProofData * toBeEval);
    void genProof(const ProofData * toBeEval);
    void genGoals(const GoalData * toBeEval);

    Q_INVOKABLE int evalProof(const ProofData *toBeEval, const GoalData *gls, ProofModel *pm);
    Q_INVOKABLE void saveProof(const QString &name,  const ProofData *toBeSaved, const GoalData *gls);
    Q_INVOKABLE void openProof(const QString &name, ProofData *openTo, GoalData *gls);
    Q_INVOKABLE void wasmOpenProof(ProofData *open, GoalData *gls);
    Q_INVOKABLE void wasmSaveProof(const ProofData *pd, const GoalData *gls);
    Q_INVOKABLE void smartPaste(ProofData *pd, ProofModel *pm);
    Q_INVOKABLE void smartCopy(const ProofData *pd, const QVariantList &selectedIndices);

    // IDBFS persistence — WASM only, no-ops on desktop
    Q_INVOKABLE void autoSave(const ProofData *pd, const GoalData *gls);
    Q_INVOKABLE void autoLoad(ProofData *openTo, GoalData *gls);
    Q_INVOKABLE bool isIdbfsReady() const;

    // Persistence warning banner — shown once, then dismissed to localStorage
    Q_INVOKABLE bool shouldShowPersistenceWarning() const;
    Q_INVOKABLE void dismissPersistenceWarning();

    proof_t *getCProof() const;
    vec_t *getReturns() const;

    QHash<QString,int> rulesMap;
    QHash<int,QString> reverseRulesMap;

    // Converts a C engine rule ID back to (category, index) pair used by the
    // UI combo boxes.  Returns {-1,-1} for structural tokens (-1, -2, etc.).
    static QPair<int,int> getCategoryAndIndex(int engineRuleId) {
        if (engineRuleId >= 0  && engineRuleId <= 9)  return {0, engineRuleId};
        if (engineRuleId >= 10 && engineRuleId <= 20) return {1, engineRuleId - 10};
        if (engineRuleId >= 21 && engineRuleId <= 29) return {2, engineRuleId - 21};
        if (engineRuleId >= 30 && engineRuleId <= 33) return {3, engineRuleId - 30};
        if (engineRuleId >= 34 && engineRuleId <= 37) return {4, engineRuleId - 34};
        return {-1, -1};
    }

signals:

    void evalTextChanged();
    void autoSaveStatusChanged();
    void errorOccurred(const QString &message);
    void smartPasteStarted();
    void smartPasteDone();


private:
    proof_t * cProof;
    vec_t * returns;
    struct connectives_list m_conns;   // connectives chosen by genProof()
//    QHash<QString,int> rulesMap;
//    QHash<int,QString> reverseRulesMap;
    QString m_evalText;
    QString m_autoSaveStatus;
    QString m_lastError;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H
