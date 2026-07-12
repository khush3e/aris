/* Class to model Proof Data (minus goals) to be viewed/interacted with in QML.

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
#ifndef PROOFMODEL_H
#define PROOFMODEL_H

#include <QAbstractListModel>
#include "proofdata.h"

class ProofModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ProofData* lines READ lines WRITE setlines)
    Q_PROPERTY(int premiseCount READ premiseCount
               NOTIFY premiseCountChanged)

public:
    explicit ProofModel(QObject *parent = nullptr);

    enum {
        LineRole = Qt::UserRole,
        TextRole,
        TypeRole,
        SubRole,
        SubStartRole,
        SubEndRole,
        IndentRole,
        RefsRole,
        ErrorRole,      // carries pErrorMsg — "errMsg" in QML
        RuleCategoryRole,   // int 0-4 — locale-invariant outer combo index
        RuleIndexRole       // int 0-N — locale-invariant inner combo index
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    ProofData *lines() const;
    void setlines(ProofData *newLines);
    Q_INVOKABLE void updateLines();
    Q_INVOKABLE void updateRefs(int ln, bool op);
    Q_INVOKABLE void clearErrors();  // reset ErrorRole on every row

    // premiseCount — computed from the model data; read-only from QML.
    int  premiseCount() const;

    
    Q_INVOKABLE bool toggleLineType(int row);

    Q_INVOKABLE void recomputePremiseCount();

signals:
    void premiseCountChanged(int n);

private:
    void setPremiseCount(int n);
    ProofData *mLines;
    int  mPremiseCount = 1;

    static QString canonicalName(int cat, int idx);

    // Returns a map from canonical English rule name -> {cat, idx}.
    struct RulePos { int cat; int idx; };
    static const QHash<QString, RulePos> &rulePosMap();
};

#endif // PROOFMODEL_H
