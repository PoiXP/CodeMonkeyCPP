#pragma once

#include <vector>
#include <algorithm>

template <class TNodeData, class TLinkData>
class Graph
{
public:
  class Node;
  class LinkArray;
  typedef TLinkData Link;
  
  class MatchNode
  {
  public:
    virtual ~MatchNode() {}
    virtual bool Match(const Node* node) = 0;
  };

  class SortCriteria
  {
  public:
    virtual ~SortCriteria() {}
    virtual bool IsLess(const Node* node1, const TLinkData& link1, const Node* node2, const TLinkData& link2) = 0;
  };

  class LinkArray
  {
  public:
    unsigned int      GetCount() const { return m_Links.size(); }
    const Node*       GetNode(unsigned int index) const { return m_Links[index].node; }
    Node*             GetNode(unsigned int index) { return m_Links[index].node; }
    const TLinkData&  GetLink(unsigned int index) const { return m_Links[index].data; }
    TLinkData&        GetLink(unsigned int index) { return m_Links[index].data; }
    unsigned int      AddLink(Node* node, const TLinkData& linkInfo)
    {
      Link link;
      link.node = node;
      link.data = linkInfo;
      m_Links.push_back(link);
      return m_Links.size() - 1;
    }
    bool              FindNode(MatchNode* match, unsigned int& linkId) const
    {
      for(unsigned int i = 0; i < GetCount(); i++)
      {
        if (match->Match( GetNode(i) ))
        {
          linkId = i;
          return true;
        }
      }
      return false;
    }
    bool              FindNode(const Node* node, unsigned int& linkId) const
    {
      for(unsigned int i = 0; i < GetCount(); i++)
      {
        if (GetNode(i) == node)
        {
          linkId = i;
          return true;
        }
      }
      return false;
    }
    void Sort(SortCriteria* criteria)
    {
      SortFunctionMapping sorter(criteria);
      std::sort(m_Links.begin(), m_Links.end(), sorter);
    }
  private:
    struct Link
    {
      Node*      node;
      TLinkData  data;  
    };
    typedef std::vector<Link> LinkVector;
    LinkVector m_Links;

    class SortFunctionMapping
    {
    public:
      SortFunctionMapping(SortCriteria* criteria): m_Criteria(criteria) {}
      bool operator()(const Link& a, const Link& b)
      {
        return m_Criteria->IsLess(a.node, a.data, b.node, b.data);
      }
      SortCriteria* m_Criteria;
    };
  };

  class Node
  {
  public:
    Node() {}
    Node(const TNodeData& data) : m_Data(data) { }

    TNodeData& GetData() { return m_Data; }
    const TNodeData& GetData() const { return m_Data; }

    LinkArray& GetChildren() { return m_Children; }
    const LinkArray& GetChildren() const { return m_Children; }

    LinkArray& GetRefBy() { return m_RefBy; }
    const LinkArray& GetRefBy() const { return m_RefBy; }
  private:
    TNodeData m_Data;
    LinkArray m_Children;
    LinkArray m_RefBy;
  };

  Graph(unsigned int nodesAmount = 0)
  {
    m_Nodes.reserve(nodesAmount);
    // Adding head
    CreateNode( TNodeData() );
  }
  virtual ~Graph()
  {
    for(NodeList::iterator it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
    {
      delete (*it);
    }
    m_Nodes.clear();
  }
  const Node* GetHead() const { return m_Nodes[0]; }
  Node* GetHead() { return m_Nodes[0]; }
  const Node* GetNode(unsigned int index) const { return m_Nodes[index]; }
  Node*       GetNode(unsigned int index) { return m_Nodes[index]; }
  unsigned int GetNodeCount() const { return m_Nodes.size(); }

  Node*       CreateNode(const TNodeData& data)
  {
    m_Nodes.push_back(new Node(data));
    return GetNode( GetNodeCount() - 1 );
  }
  void AddDependency(Node* fromNode, Node* toNode)
  {
    unsigned int childIndex = FindNodeIndex(fromNode->GetChildren(), toNode);
    ApplyDependency(fromNode->GetChildren().GetLink(childIndex));
    unsigned int refByIndex = FindNodeIndex(toNode->GetRefBy(), fromNode);
    ApplyDependency(toNode->GetRefBy().GetLink(refByIndex));
  }
  template <class T> void VisitAll(T func) const
  {
    VisitNode(GetHead(), func);
  }
protected:
  virtual void ApplyDependency(TLinkData& data) = 0;
private:
  template <class T> void VisitNode(const Node* node, T func)  const
  {
    func(node);
    for (unsigned int i = 0u; i < GetNodeCount(); i++)
    {
      VisitNode( node->GetChildren().GetNode(i), func);
    }
  }
  unsigned int FindNodeIndex(LinkArray& links, Node* node)
  {
    for (unsigned int i = 0u; i < links.GetCount(); i++)
    {
      if (links.GetNode(i) == node)
      {
        return i;
      }
    }
    return links.AddLink(node, LinkData());
  }
  /// All nodes
  typedef std::vector<Node*> NodeList;
  std::vector<Node*> m_Nodes;
};
