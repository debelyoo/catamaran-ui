#ifndef HIERARCHICALIDENTIFIER_H
#define HIERARCHICALIDENTIFIER_H

#include <QString>

template <char separator>
class HierarchicalIdentifier
{
public:
    HierarchicalIdentifier(const QString &address);
    //HierarchicalIdentifier(const HierarchicalIdentifier &hi);
    ~HierarchicalIdentifier();
    bool hasParent() const;
    bool isValid() const; 
    const QString &id() const;

    const HierarchicalIdentifier<separator> &parent(int level=-1) const;
    int level() const;

    operator QString() const;

private:
    QString m_fullId;
    QString m_me;
    HierarchicalIdentifier<separator> *m_parent;
    int m_level;

    void parse();
};
#include <QDebug>
template <char separator>
HierarchicalIdentifier<separator>::HierarchicalIdentifier(const QString &address):
    m_fullId(address),
    m_me(""),
    m_parent(NULL)
{
    parse();
    m_level = this->level();
}

/*
template <char separator>
HierarchicalIdentifier<separator>::HierarchicalIdentifier(const HierarchicalIdentifier<separator> &hi)
{
    m_fullId = hi.m_fullId;
    m_me = hi.m_me;
    qDebug() << "id="<<m_me<<" fullId="<<m_fullId;
    if(hi.m_parent){
        m_parent = new HierarchicalIdentifier<separator>(*hi.m_parent);
    }else{
        m_parent = NULL;
    } 
}*/

template <char separator>
HierarchicalIdentifier<separator>::~HierarchicalIdentifier()
{
    if(m_parent){
        delete m_parent;
    }
}

template <char separator>
bool HierarchicalIdentifier<separator>::hasParent() const
{
    return m_parent != NULL;
}

template <char separator>
bool HierarchicalIdentifier<separator>::isValid() const
{
    return !m_fullId.isEmpty();
}

template <char separator>
const HierarchicalIdentifier<separator> &HierarchicalIdentifier<separator>::parent(int level) const
{
    if(level >= 0 && level < m_level){
        if(level == m_level-1){
            return *m_parent;
        }else{
            return m_parent->parent(level);
        }
    }
    return *m_parent;
}

template <char separator>
int HierarchicalIdentifier<separator>::level() const
{
    if(hasParent()){
        return m_parent->level()+1;
    }else{
        return 0;
    }
}

template <char separator>
const QString &HierarchicalIdentifier<separator>::id() const
{
    return m_me;
}

template <char separator>
HierarchicalIdentifier<separator>::operator QString() const
{
    return m_fullId;
}

template <char separator>
void HierarchicalIdentifier<separator>::parse()
{
    if(!m_fullId.isEmpty()){
        int pos = m_fullId.lastIndexOf(separator);
        if(pos > 0){
            m_parent = new HierarchicalIdentifier<separator>(m_fullId.left(pos));
            m_me = m_fullId.mid(pos + 1);
        }else{
            m_me = m_fullId;
            m_parent = NULL;
        }
    }else{
        m_me = m_fullId;
        m_parent = NULL;
    }
}

typedef HierarchicalIdentifier<'.'> SensorAddress;

#endif // HIERARCHICALIDENTIFIER_H
