#pragma once
#include"CoreTypes.h"	
#include <vector>
#include<memory>
#include<array>
#include<map>
namespace Structures
{
    //this template is used by the quadtree to allow objects to return their custom rect definitions
    template<class T>
    struct FRectAccessor
    {
        static const Rect GetRect(const T& Obj);
    };

    //default rect access for arect, just return therect itself
    template<>
    struct FRectAccessor<Rect>
    {
        static const Rect GetRect(const Rect& Obj) { return Obj; };
    };
    
    template<typename item_t>
    class Quadtree
    {
    public:
        using rectaccessor_t = typename FRectAccessor<item_t>;

    private:
        struct ItemPak
        {
            ItemPak(const item_t& item) :
                m_Item{ item }, m_OldRect{ GetCurrnetRect() }
            {

            }
            item_t m_Item;
            Rect m_OldRect;
            const Rect& GetCurrnetRect()const
            {
                return rectaccessor_t::GetRect(m_Item);
            }

            bool Update()
            {
                Rect newRect = GetCurrnetRect();
                if (m_OldRect != newRect)
                {
                    m_OldRect = std::move(newRect);
                    return true;
                }
                return false;
            }
        };
        class QuadtreeNode
        {
        public:
            QuadtreeNode(Quadtree* ParentTree, Rect bounds, size_t level = 0) :
                m_Bounds(bounds),
                m_Level(level),
                m_ParentTree{ ParentTree }
            {}

            // Insert an item into the quadtree
            bool insert(const item_t& item)
            {
                if (IsSplitted())
                {
                    int index = GetIndex(item);
                    if (index != -1)
                    {
                        return m_Children[index]->insert(item);
                    }
                    else
                    {
                        return false;
                    }
                }

                m_Items.push_back(ItemPak(item));

                if (ShouldSplit() && CanSplit() )
                {
                    Split();
                    for (auto& i : m_Items)
                    {
                        int index = GetIndex(i);
                        if (index == -1)
                        {
                            m_ParentTree->ReInsert(i.m_Item);
                        }
                        else
                        {
                            m_Children[index]->insert(i.m_Item);
                        }
                    }
                    m_Items.clear();
                }
                return true;
            }

            // Query rectangles within a given range
            std::vector<item_t> query(const Rect& range)
            {
                std::vector<item_t> result;

                if (!Intersects(m_Bounds, range))
                {
                    return result;
                }

                if (IsSplitted())
                {
                    for (size_t i = 0; i < 4; i++)
                    {
                        auto res = m_Children[i]->query(range);
                        result.insert(result.end(), res.begin(), res.end());
                    }
                }
                else
                {
                    for (auto it = m_Items.begin(); it != m_Items.end();)
                    {
                        bool bErase = false;
                        if (it->Update())
                        {
                            if (!Intersects(m_Bounds, it->m_OldRect))
                            {
                                m_ParentTree->ReInsert(it->m_Item);
                                bErase = true;
                            }
                        }
                        if (Intersects(range, it->m_OldRect))
                        {
                            result.push_back(it->m_Item);
                        }

                        if (bErase)
                        {
                            it = m_Items.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                }

                return result;
            }

            inline bool IsSplitted()const
            {
                return m_Children[0] != nullptr;
            }
            inline bool CanSplit()const
            {
                return m_Level < m_ParentTree->GetMaxLevels();
            }
            inline bool ShouldSplit()const
            {
                return m_Items.size() > m_ParentTree->GetMaxItemsPerNode();
            }
            inline const Rect& GetBounds()const
            {
                return m_Bounds;
            }
            inline std::vector<Rect> GetAllBounds()const
            {
                std::vector<Rect> res;
                if (IsSplitted())
                {
                    for (size_t i = 0; i < 4; i++)
                    {
                        auto  cres = m_Children[i]->GetAllBounds();
                        res.insert(res.end(), cres.begin(), cres.end());
                    }
                }
                else
                {
                    res.push_back(GetBounds());
                }
                return res;
            }
        private:
            Rect m_Bounds;
            size_t m_Level;
            Quadtree* m_ParentTree;

            std::vector<ItemPak> m_Items;

            std::array<std::unique_ptr<QuadtreeNode>, 4> m_Children; // Four children nodes (NW, NE, SW, SE)


            void Split()
            {
                if (IsSplitted())
                {
                    return;
                }
                float subWidth = m_Bounds.Size.x / 2.0f;
                float subHeight = m_Bounds.Size.y / 2.0f;
                float x = m_Bounds.Origin.x;
                float y = m_Bounds.Origin.y;

                m_Children[0].reset(new QuadtreeNode(m_ParentTree, Rect({ x , y + subHeight }, { subWidth, subHeight }), m_Level + 1)); // NW
                m_Children[1].reset(new QuadtreeNode(m_ParentTree, Rect({ x + subWidth, y + subHeight }, { subWidth, subHeight }), m_Level + 1)); // NE
                m_Children[2].reset(new QuadtreeNode(m_ParentTree, Rect({ x+subWidth, y }, { subWidth, subHeight }), m_Level + 1)); // SE
                m_Children[3].reset(new QuadtreeNode(m_ParentTree, Rect({ x, y }, { subWidth, subHeight }), m_Level + 1)); // SW

            }
            int GetIndex(const ItemPak& itemPak) const // Get the index of the child node for an item
            {
                
                const Rect& rect = itemPak.m_OldRect;//get the item rect

                int index = -1;
                float verticalMidpoint = m_Bounds.Origin.x + (m_Bounds.Size.x / 2.0f);
                float horizontalMidpoint = m_Bounds.Origin.y + (m_Bounds.Size.y / 2.0f);

                bool Top = rect.Origin.y > horizontalMidpoint ;
                bool Right = rect.Origin.x > verticalMidpoint;

                if (Top)
                {
                    index = Right ? 1 : 0;
                }
                else
                {
                    index = Right ? 2 : 3;
                }
                return index;
            }
        };

    public:

        Quadtree(Rect bounds, size_t maxLevels = 8, size_t maxItemsPerNode = 10) :
            m_Root{ this ,std::move(bounds),0 },
            m_MaxItemsPerNode{ maxItemsPerNode },
            m_MaxLevels{ maxLevels }
        {}

        // Insert a rectangle into the quadtree
        bool insert(const item_t& item)
        {
            if (!Intersects(m_Root.GetBounds(), rectaccessor_t::GetRect(item)))
            {
                return false;
            }
            return m_Root.insert(item);
        }

        // Query rectangles within a given range
        std::vector<item_t> query(const Rect& range)
        {
            auto res = m_Root.query(range);
            for (auto& i : m_PendingReInsert) {
                insert(i);
            }
            m_PendingReInsert.clear();
            return res;
        }

        inline size_t GetMaxLevels()const { return m_MaxLevels; };
        inline size_t GetMaxItemsPerNode()const { return m_MaxItemsPerNode; };
        inline const Rect& GetBounds()const { return m_Root.GetBounds(); }
        std::vector<Rect> GetAllBounds()const
        {
            return m_Root.GetAllBounds();
        }
    private:
        QuadtreeNode m_Root;
        size_t m_MaxItemsPerNode;
        size_t m_MaxLevels;

        void ReInsert(const item_t item)
        {
            m_PendingReInsert.push_back(item);
        }
        std::vector<item_t> m_PendingReInsert;

        friend class QuadtreeNode;
    };
}