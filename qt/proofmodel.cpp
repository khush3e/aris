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
#include "proofmodel.h"

// Static helpers — locale-invariant rule name ↔ (category, index) mapping


// canonical rule names ordered exactly as the combo2 arrays in ProofArea.qml
static const char * const kRuleNames[][11] = {
    // cat 0 — Inference (10 rules)
    {"Modus Ponens","Addition","Simplification","Conjunction",
     "Hypothetical Syllogism","Disjunctive Syllogism","Excluded middle",
     "Constructive Dilemma","XOR Introduction","XOR Elimination", nullptr},
    // cat 1 — Equivalence (11 rules)
    {"Implication","DeMorgan","Association","Commutativity","Idempotence",
     "Distribution","Equivalence","Double Negation","Exportation",
     "Subsumption","Contrapositive"},
    // cat 2 — Predicate (9 rules)
    {"Universal Generalization","Universal Instantiation",
     "Existential Generalization","Existential Instantiation",
     "Bound Variable Substitution","Null Quantifier","Prenex",
     "Identity","Free Variable Substitution"},
    // cat 3 — Miscellaneous (4 rules)
    {"Lemma","Subproof","Sequence","Induction"},
    // cat 4 — Boolean (4 rules)
    {"Boolean Identity","Boolean Negation","Boolean Dominance","Symbol Negation"}
};
static const int kRuleCounts[] = {10, 11, 9, 4, 4};

/*static*/ const QHash<QString, ProofModel::RulePos> &ProofModel::rulePosMap()
{
    static QHash<QString, RulePos> m;
    if (m.isEmpty()) {
        for (int cat = 0; cat < 5; ++cat) {
            for (int idx = 0; idx < kRuleCounts[cat]; ++idx) {
                m.insert(QString::fromLatin1(kRuleNames[cat][idx]), RulePos{cat, idx});
            }
        }
    }
    return m;
}

/*static*/ QString ProofModel::canonicalName(int cat, int idx)
{
    if (cat < 0 || cat > 4) return QString();
    if (idx < 0 || idx >= kRuleCounts[cat]) return QString();
    return QString::fromLatin1(kRuleNames[cat][idx]);
}


ProofModel::ProofModel(QObject *parent)
    : QAbstractListModel(parent), mLines(nullptr)
{
}

// Return the number of rows in the model.
int ProofModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !mLines)
        return 0;

    return mLines->lines().size();
}

// Return data corresponding to a particular index and role(enum)
QVariant ProofModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const ProofLine someLine = mLines->lines().at(index.row());

    switch (role){
    case LineRole:
        return QVariant(someLine.pLine);
    case TextRole:
        return QVariant(someLine.pText);
    case TypeRole:
        return QVariant(someLine.pType);
    case SubRole:
        return QVariant(someLine.pSub);
    case SubStartRole:
        return QVariant(someLine.pSubStart);
    case SubEndRole:
        return QVariant(someLine.pSubEnd);
    case IndentRole:
        return QVariant(someLine.pInd);
    case RefsRole: {
        QList<QVariant> ret;
        for (int x: someLine.pRefs)
            ret.append(x);
        return ret;
    }
    case ErrorRole:
        return QVariant(someLine.pErrorMsg);
    case RuleCategoryRole:
        return QVariant(someLine.pRuleCategory);
    case RuleIndexRole:
        return QVariant(someLine.pRuleIndex);
    case CollapsedRole:
        return QVariant(someLine.pCollapsed);
    case HiddenRole:
        return QVariant(isHiddenByCollapse(index.row()));
    }
    return QVariant();
}

// Set/Change/Edit data corresponding to a particular index and role(enum) with the given value
bool ProofModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!mLines)
        return false;

    ProofLine someLine = mLines->lines().at(index.row());

    switch (role){
    case LineRole:
        someLine.pLine = value.toInt();
        break;
    case TextRole:
        someLine.pText = value.toString();
        break;
    case TypeRole: {
        someLine.pType = value.toString();
        // Derive the integer combo position from the canonical English name so
        // QML combo boxes always have a locale-invariant index to bind to.
        auto it = rulePosMap().constFind(someLine.pType);
        if (it != rulePosMap().constEnd()) {
            someLine.pRuleCategory = it->cat;
            someLine.pRuleIndex    = it->idx;
        } else {
            // Structural token ("premise", "sf", "subproof", "choose") or unknown.
            someLine.pRuleCategory = -1;
            someLine.pRuleIndex    = -1;
        }
        break;
    }
    case SubRole:
        someLine.pSub = value.toBool();
        break;
    case SubStartRole:
        someLine.pSubStart = value.toBool();
        break;
    case SubEndRole:
        someLine.pSubEnd = value.toBool();
        break;
    case IndentRole:
        someLine.pInd = value.toInt();
        break;
    case RefsRole:
        someLine.pRefs.clear();
        {
            QVariantList temp = value.toList();
            for (const QVariant &x: qAsConst(temp))
                someLine.pRefs.append(x.toInt());
        }
        break;
    case ErrorRole:
        // Call setErrorAt directly, then emit dataChanged ourselves.
        mLines->setErrorAt(index.row(), value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case RuleCategoryRole:
        someLine.pRuleCategory = value.toInt();
        someLine.pRuleIndex = mLines->lines().at(index.row()).pRuleIndex;
        {
            const QString name = canonicalName(someLine.pRuleCategory, someLine.pRuleIndex);
            if (!name.isEmpty()) someLine.pType = name;
        }
        break;
    case RuleIndexRole:
        someLine.pRuleIndex = value.toInt();
        someLine.pRuleCategory = mLines->lines().at(index.row()).pRuleCategory;
        {
            const QString name = canonicalName(someLine.pRuleCategory, someLine.pRuleIndex);
            if (!name.isEmpty()) someLine.pType = name;
        }
        break;
    case CollapsedRole:
        // Handled by toggleCollapsed() — direct writes are a no-op.
        return false;
    case HiddenRole:
        // Read-only computed role.
        return false;
    }

    if (mLines->setLineAt(index.row(),someLine)) {
        if (role == TypeRole || role == RuleCategoryRole || role == RuleIndexRole) {
            emit dataChanged(index, index, {TypeRole, RuleCategoryRole, RuleIndexRole});
        } else {
            emit dataChanged(index, index, {role});
        }
        return true;
    }
    return false;
}

Qt::ItemFlags ProofModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable;
}

// Define role names.
QHash<int, QByteArray> ProofModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[LineRole] = "line";
    names[TextRole] = "lText";
    names[TypeRole] = "type";
    names[SubRole] = "sub";
    names[SubStartRole] = "subSt";
    names[SubEndRole] = "subEnd";
    names[IndentRole] = "ind";
    names[RefsRole] = "refs";
    names[ErrorRole] = "errMsg";
    names[RuleCategoryRole] = "ruleCategory";
    names[RuleIndexRole]    = "ruleIndex";
    names[CollapsedRole]    = "collapsed";
    names[HiddenRole]       = "hidden";
    return names;
}

ProofData *ProofModel::lines() const
{
    return mLines;
}

// Update the model with the new proof line(s)
void ProofModel::setlines(ProofData *newLines)
{
    beginResetModel();

    if (mLines)
        mLines->disconnect(this);

    mLines = newLines;

    if (mLines){
        connect(mLines,&ProofData::preLineInsert,this,[=](int index){
            beginInsertRows(QModelIndex(),index,index);
        });
        connect(mLines,&ProofData::postLineInsert,this,[=](){
            endInsertRows();
            recomputePremiseCount();
        });
        connect(mLines,&ProofData::preLineRemove,this,[=](int index){
            beginRemoveRows(QModelIndex(),index,index);
        });
        connect(mLines,&ProofData::postLineRemove,this,[=](){
            endRemoveRows();
            recomputePremiseCount();
        });
    }

    endResetModel();
    recomputePremiseCount();
}

// TODO: Use model indices directly in QML, no need to update lines that way

// Update line number roles after insertion and/or removal of a proof line
void ProofModel::updateLines()
{
    for (int i = 0; i < mLines->lines().size(); ++i) {
        setData(index(i,0),i+1,LineRole);
    }
}

// Update relevant reference roles after insertion and/or removal of a proof line
void ProofModel::updateRefs(int ln, bool op)
{
    qDebug() << "[ProofModel] updateRefs called with ln =" << ln << "op =" << op;
    for (int i = ln+1; i < mLines->lines().size(); i++){
        QList<int> refs = mLines->lines().at(i).pRefs;
        bool changed = false;

        for (int ii = 1; ii < refs.size(); ii++){
            if (op){
                if (refs[ii] >= (ln+1)) {
                    qDebug() << "[ProofModel] row" << i << "incrementing ref" << refs[ii] << "to" << refs[ii] + 1;
                    refs[ii]++;
                    changed = true;
                }
            }
            else{
                if (refs[ii] == (ln + 1)) {
                    qDebug() << "[ProofModel] row" << i << "removing ref" << refs[ii];
                    refs.removeAt(ii);
                    changed = true;
                    ii--; // Adjust index after removal
                }
                else if (refs[ii] > (ln + 1)) {
                    qDebug() << "[ProofModel] row" << i << "decrementing ref" << refs[ii] << "to" << refs[ii] - 1;
                    refs[ii]--;
                    changed = true;
                }
            }
        }
        
        if (changed) {
            QList<QVariant> ret;
            for (int x: refs)
                ret.append(x);
            setData(index(i,0),ret,RefsRole);
            qDebug() << "[ProofModel] setData called for row" << i << "new refs:" << ret;
        }
    }
}

// Clear all inline error messages in the model (called before each evaluation).
void ProofModel::clearErrors()
{
    if (!mLines) return;
    const int n = mLines->lines().size();
    for (int i = 0; i < n; i++) {
        mLines->setErrorAt(i, QString());
        emit dataChanged(index(i, 0), index(i, 0), {ErrorRole});
    }
}

// Scrub every reference to lineNum (1-based display number) from ALL rows.
// Called before physically moving a line so no other line ends up with a
// stale ref that becomes a self-reference after the remove+reinsert cycle.
// updateRefs(ln, false) only iterates rows AFTER ln — this covers all rows.
void ProofModel::clearRefsToLine(int lineNum)
{
    if (!mLines) return;
    const int n = mLines->lines().size();
    for (int i = 0; i < n; i++) {
        ProofLine ln = mLines->lines().at(i);
        bool changed = false;
        // pRefs[0] is always the sentinel -1; start at index 1.
        for (int j = ln.pRefs.size() - 1; j >= 1; j--) {
            if (ln.pRefs[j] == lineNum) {
                ln.pRefs.removeAt(j);
                changed = true;
            }
        }
        if (changed) {
            mLines->setLineAt(i, ln);
            emit dataChanged(index(i, 0), index(i, 0), {RefsRole});
        }
    }
}

// premiseCount property 

int ProofModel::premiseCount() const
{
    return mPremiseCount;
}

void ProofModel::setPremiseCount(int n)
{
    if (mPremiseCount == n) return;
    mPremiseCount = n;
    emit premiseCountChanged(n);
}

// Toggle a single row between "premise" and "choose".
// Clears the row's refs to {-1} and recomputes premiseCount from data.
// Returns false if the row is out of range or is a subproof/sf line.
bool ProofModel::toggleLineType(int row)
{
    if (!mLines || row < 0 || row >= mLines->lines().size())
        return false;

    ProofLine ln = mLines->lines().at(row);

    // Refuse to touch structural subproof lines.
    if (ln.pType == "sf" || ln.pType == "subproof")
        return false;

    if (ln.pType == "premise") {
        ln.pType = "choose";
        ln.pRefs = {-1};
        // Entering "choose" state: clear rule integers so the UI starts fresh.
        ln.pRuleCategory = -1;
        ln.pRuleIndex    = -1;
        if (!mLines->setLineAt(row, ln)) return false;
        emit dataChanged(index(row, 0), index(row, 0), {TypeRole, RefsRole, RuleCategoryRole, RuleIndexRole});
        recomputePremiseCount();
    } else {
        // Any non-premise, non-subproof type → "premise"
        ln.pType = "premise";
        ln.pRefs = {-1};
        ln.pRuleCategory = -1;
        ln.pRuleIndex    = -1;
        if (!mLines->setLineAt(row, ln)) return false;
        emit dataChanged(index(row, 0), index(row, 0), {TypeRole, RefsRole, RuleCategoryRole, RuleIndexRole});
        recomputePremiseCount();
    }
    return true;
}

// Rescan all rows and recompute premiseCount from scratch.
// A "premise" is any leading row whose type == "premise" (contiguous from row 0).
void ProofModel::recomputePremiseCount()
{
    if (!mLines) return;
    int count = 0;
    const int n = mLines->lines().size();
    for (int i = 0; i < n; i++) {
        if (mLines->lines().at(i).pType == "premise")
            count++;
        else
            break;
    }
    setPremiseCount(count);
}

// Returns true if `row` is inside a currently collapsed subproof block.
//
// Subproof structure (indent units = 20px each level):
//   parent scope at indent X
//     sf row (pSubStart=true)  at indent X+20   ← same as content rows
//     content rows             at indent X+20
//     end row (pSubEnd=true)   at indent X
//
// Because the sf row sits at the SAME indent as the content rows, we walk
// backward skipping only rows that are DEEPER (pInd > myInd), then check
// the first pSubStart row at the same indent — if collapsed, we are hidden.
bool ProofModel::isHiddenByCollapse(int row) const
{
    if (!mLines || row <= 0) return false;
    const QVector<ProofLine> ls = mLines->lines();
    const int myInd = ls.at(row).pInd;

    // Rows at indent 0 can never be inside a subproof.
    if (myInd <= 0) return false;

    for (int i = row - 1; i >= 0; --i) {
        const ProofLine &pl = ls.at(i);

        // Skip rows that are deeper — they are inside a nested subproof
        // and cannot be the opener for the current row.
        if (pl.pInd > myInd) continue;

        // A subproof-start at the same indent level is the direct opener.
        // If it is collapsed → this row is hidden; if expanded → visible.
        if (pl.pSubStart && pl.pInd == myInd)
            return pl.pCollapsed;

        // Hit a row at a strictly lower indent without finding an sf opener
        // → we are not inside any subproof at this level.
        if (pl.pInd < myInd)
            return false;

        // Same-indent non-sf row — keep walking backward.
    }
    return false;
}

// Toggle the pCollapsed flag on a subproof-start (sf) row and notify QML.
// Emits CollapsedRole on the sf row (updates the chevron ▶/▼) and
// HiddenRole on every row inside the block so their height/visible update.
void ProofModel::toggleCollapsed(int row)
{
    if (!mLines || row < 0 || row >= mLines->lines().size()) return;

    const QVector<ProofLine> ls = mLines->lines();
    const ProofLine &sf = ls.at(row);

    // Only act on subproof-start rows.
    if (!sf.pSubStart) return;

    const bool newVal = !sf.pCollapsed;
    mLines->setCollapsedAt(row, newVal);

    // Find the matching subproof-end.
    // The sf row is at indent sfInd; its matching end row is at sfInd - 20.
    const int totalRows = mLines->lines().size();
    const int sfInd     = sf.pInd;
    int endRow = row;
    for (int i = row + 1; i < totalRows; ++i) {
        const ProofLine &pl = mLines->lines().at(i);
        endRow = i;
        if (pl.pSubEnd && pl.pInd == (sfInd - 20))
            break;
    }

    // Notify QML: chevron on the sf row, hidden state on every inner row.
    emit dataChanged(index(row, 0), index(row, 0), {CollapsedRole});
    if (endRow > row)
        emit dataChanged(index(row + 1, 0), index(endRow, 0), {HiddenRole});
}
