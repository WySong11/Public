using Assets.InGame.Scripts.Deployment;
using Assets.InGame.UI.Deployment.Scripts;
using EnhancedScrollerDemos.MultipleCellTypesDemo;
using EnhancedUI.EnhancedScroller;
using Spine;
using System;
using System.CodeDom;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.UIElements;

namespace Assets.InGame.UI.Scripts.Deployment
{
    public class DeploymentCharacterListWindow : MonoBehaviour, IEnhancedScrollerDelegate
    {
        protected int m_listCount;
        protected float m_sizeDelta;

        [Header("= 한 줄에 표시돼는 View의 수 =")]
        public int m_rowViewCount = 2;

        [Header("= 최소 라인 수 =")]
        public int m_columnViewCount = 0;
        public Vector3 m_viewScale = Vector3.one;

        public GameObject m_leftBg;
        public GameObject m_leftScrollObj;
        public EnhancedScroller m_leftScroller;

        public GameObject m_rightBg;
        public GameObject m_rightScrollObj;
        public EnhancedScroller m_rightScroller;

        public DeploymentCharacterGroupView m_groupView;
        public DeploymentCharactertContainer m_containerView;

        public RectTransform m_leftInfoPosition;
        public RectTransform m_rightInfoPosition;

        public GameObject m_leftDim;
        public GameObject m_rightDim;

        public DeploymentFieldWindow m_fieldWindow;

        [Header("= True 라면 콘테이너가 스크롤 영역 안에서도 보임.")]
        [Header("= False  면 콘테이너가 스크롤 영역에 안 보임.")]
        public bool m_viewContainerInScrollRect = false;

        [Header("= 더블 클릭 간격. CharacterInfoWindow 보여줌 =")]
        public float m_clickTime = 0.3f;

        public bool m_isLeft { get; private set; }

        [Header("= 테스트용 =")]
        public bool m_testIsLeft = false;
        public bool m_testIsRight = false;

        // 캐릭터 배치 할 때 사용
        private bool m_isDragging = false;

        // 캐릭터 배치 할 때
        public Action<int, int, Vector2> m_actionBeginDrag;
        public Action<Vector2> m_actionDragging;
        public Action<Vector2, bool> m_actionEndDrop;

        // 캐릭터 정보 볼 때
        public Action<int> m_actionDoubleClick;

        // 배치 캐릭터 삭제
        public Action<int, int> m_actionRecall;

        // 배치한 캐릭터 이동 할 때
        public Action<Vector2> m_actionBeginMove;
        public Action<Vector2, bool, bool> m_actionMoving;
        public Action<Vector2, int, bool> m_actionEndMove;

        private bool m_isMoving = false;
        private int m_selectIndex = -1;

        private List<UnitListData> m_listData = new List<UnitListData>();
        private TeamData m_teamData = new TeamData();

        private int m_turnCost;
        private int m_useCost;

        public DeploymentUnitRecallButton m_recallButtonPrefab;
        private List<DeploymentUnitRecallButton> m_listRecallButton = new List<DeploymentUnitRecallButton>();

        // Use this for initialization
        void Start()
        {
        }

        // Update is called once per frame
        void Update()
        {
        }

        void OnEnable()
        {
        }

        void OnDisable()
        {
            foreach(var item in m_listRecallButton)
            {
                Destroy(item.gameObject);
            }

            m_listRecallButton.Clear();
        }

        void Awake()
        {
            m_fieldWindow.m_actionBeginMove = (outPos) =>
            {
                m_isMoving = false;
                m_actionBeginMove?.Invoke(outPos);
            };

            m_fieldWindow.m_actionMoving = (outPos) =>
            {
                if (m_isMoving == true)
                {
                    bool scrollArena = IsScrollArena(outPos);
                    bool myArena = IsMyArena(outPos);

                    m_containerView.SetVisible(scrollArena);
                    m_containerView.SetPosition(outPos);

                    m_actionMoving?.Invoke(outPos, myArena, scrollArena);
                }
            };

            m_fieldWindow.m_actionEndMove = (outPos) =>
            {
                if (m_isMoving == true)
                {
                    m_isMoving = false;
                    m_containerView.SetVisible(false);
                    m_actionEndMove?.Invoke(outPos, m_selectIndex, IsMyArena(outPos) == true && IsScrollArena(outPos) == false);
                }
            };
        }

        public void SetVisible(bool value, bool isLeft = true)
        {
            gameObject.SetActive(value);

            if (value == true)
            {
                m_isLeft = m_testIsLeft == true ? m_testIsLeft : isLeft;
                m_isLeft = m_testIsRight == true ? m_testIsRight : isLeft;

                if (m_isLeft == true)
                {
                    m_leftScroller.Delegate = this;
                }
                else
                {
                    m_rightScroller.Delegate = this;
                }

                m_leftBg.SetActive(m_isLeft == true);
                m_rightBg.SetActive(m_isLeft == false);

                m_leftScrollObj.SetActive(m_isLeft == true);
                m_rightScrollObj.SetActive(m_isLeft == false);

                m_leftDim.SetActive(m_isLeft == false);
                m_rightDim.SetActive(m_isLeft == true);

                m_isDragging = false;
                m_isMoving = false;
                m_selectIndex = -1;

                m_containerView.SetVisible(false);

                m_listRecallButton = new List<DeploymentUnitRecallButton>();
            }            
        }

        public void SortCharacterList(TeamData inTeamData, int inTurnCost, int inUseCost)
        {
            m_teamData = inTeamData;
            m_turnCost = inTurnCost;
            m_useCost = inUseCost;

            int prevListCount = m_listData.Count;

            m_listData.Clear();

            foreach (UnitSetInRawData SetInData in CharacterFactory.m_instance.m_list_unitSetIn)
            {
                UnitRawData rawData = CharacterFactory.m_instance.GetUnitData(SetInData.m_ids[0]);

                int maxCount = GameHelper.m_instance.GetLimitRequlationCount(rawData.m_rare);
                int count = m_teamData.GetUnitSetInCount(SetInData.m_id, GameHelper.m_instance.GetTurn(), out List<int> numberList);

                if (count < maxCount)
                {
                    // 갯수 갱신
                    for (int i=0; i<maxCount-count; i++)
                    {
                        UnitListData tempData = new UnitListData();

                        tempData.m_id = SetInData.m_id;
                        tempData.m_number = i;
                        tempData.m_data = SetInData;

                        m_listData.Add(tempData);
                    }
                }   
            }

            m_listCount = m_listData.Count;

            m_sizeDelta = m_groupView.m_characterView.m_rectTransform.sizeDelta.y;

            ReloadData(m_listCount == prevListCount);
        }

        public void RefreshList()
        {
            ReloadData(true);
        }

        int IEnhancedScrollerDelegate.GetNumberOfCells(EnhancedScroller scroller)
        {
            int count = (m_listCount + m_rowViewCount - 1) / m_rowViewCount;

            return Mathf.Max(m_columnViewCount, count);
        }

        float IEnhancedScrollerDelegate.GetCellViewSize(EnhancedScroller scroller, int dataIndex)
        {
            return m_sizeDelta;
        }

        EnhancedScrollerCellView IEnhancedScrollerDelegate.GetCellView(EnhancedScroller scroller, int dataIndex, int cellIndex)
        {
            DeploymentCharacterGroupView GroupView = scroller.GetCellView(m_groupView) as DeploymentCharacterGroupView;            
            GroupView.SetScale(m_viewScale);
            GroupView.SetScroller(GetScroller());

            for (int i = 0; i < m_rowViewCount; i++)
            {
                int index = (dataIndex * m_rowViewCount) + i;

                if (index < m_listCount)
                {
                    GroupView.SetUnitData(i, m_listData[index], m_teamData, m_turnCost, m_useCost, m_isLeft, m_viewContainerInScrollRect, m_clickTime);
                }
                else
                {
                    GroupView.SetUnitData(i, null);
                }
            }

            GroupView.m_actionBeginDrag = (outIndex, outNumber, outImage, outPos) =>
            {
                if (m_isDragging == false)
                {
                    m_containerView.SetImage(outImage, m_isLeft);
                    m_isDragging = true;

                    m_actionBeginDrag?.Invoke(outIndex, outNumber, outPos);
                }
            };

            GroupView.m_actionDragging = (outPos) =>
            {
                if (m_isDragging == true)
                {
                    bool scrollArena = IsScrollArena(outPos);

                    m_containerView.SetVisible(scrollArena);
                    m_containerView.SetPosition(outPos);

                    m_actionDragging?.Invoke(outPos);
                }
            };

            GroupView.m_actionEndDrag = (outPos) =>
            {
                if (m_isDragging == true)
                {
                    m_containerView.SetVisible(false);
                    m_isDragging = false;

                    m_actionEndDrop?.Invoke(outPos, IsMyArena(outPos) == true && IsScrollArena(outPos) == false);                    
                }
            };

            GroupView.m_actionDoubleClick = (outIndex) =>
            {
                DeploymentCharacterInfoWindow.m_instance.SetVisible(true, outIndex, m_isLeft, (m_isLeft == true) ? m_leftInfoPosition : m_rightInfoPosition );
            };

            GroupView.m_actionRecall = (outIndex, outNumber) =>
            {
                m_actionRecall?.Invoke(outIndex, outNumber);
            };

            GroupView.selected = CellViewSelected;

            return GroupView;
        }

        public void ReloadData(bool isStay = true)
        {
            float scrollPositionFactor = 0;
            if (isStay == true) scrollPositionFactor = GetScroller().NormalizedScrollPosition;
            GetScroller().ReloadData(scrollPositionFactor);
        }

        private void CellViewSelected(DeploymentCharacterListView cellView)
        {
            if (cellView == null)
            {

            }
            else
            {

            }
        }

        public EnhancedScroller GetScroller()
        {
            return (m_isLeft == true) ? m_leftScroller : m_rightScroller;
        }

        public void SetMoveCharacter(bool isMove=false, int inIndex=-1 , Vector2 inPos = new Vector2())
        {
            m_isMoving = isMove;
            m_selectIndex = inIndex;
            m_fieldWindow.SetDrag(true);

            // 콘테이너 설정. 일단 안 보이게 함.
            UnitSetInRawData tempData = CharacterFactory.m_instance.GetUnitSetInData(m_selectIndex);
            m_containerView.SetVisible(false);
            m_containerView.SetImage(CharacterFactory.m_instance.GetUnitThumbnail(tempData.m_thumbnail_name), m_isLeft);
            m_containerView.SetPosition(inPos);
        }

        public bool IsMyArena(Vector2 inPos)
        {
            GameObject tempArena = (m_isLeft == true) ? m_rightDim : m_leftDim;

            RectTransform tempRect = tempArena.GetComponent<RectTransform>();

            bool IsEnemyArena = RectTransformUtility.RectangleContainsScreenPoint(tempRect, inPos);

            tempRect = gameObject.GetComponent<RectTransform>();

            bool IsUIArena = RectTransformUtility.RectangleContainsScreenPoint(tempRect, inPos);

            return IsEnemyArena == false && IsUIArena == true;
        }

        public bool IsScrollArena(Vector2 inPos)
        {
            GameObject tempArena = (m_isLeft == true) ? m_leftBg : m_rightBg;

            RectTransform tempRect = tempArena.GetComponent<RectTransform>();

            return RectTransformUtility.RectangleContainsScreenPoint(tempRect, inPos) == true;
        }

        public void ResetDrag()
        {
            m_isDragging = false;
        }

        public void CreateRecallButton(int inCharacterID, int inNumber)
        {
            Vector2 pos = m_containerView.gameObject.transform.localPosition;

            var tempData = CharacterFactory.m_instance.GetUnitSetInData(inCharacterID);

            pos.x += tempData.m_button_x;
            pos.y += tempData.m_button_y;

            CreateRecallButton(pos, inCharacterID, inNumber);
        }

        public void CreateRecallButton(Vector2 inPos, int inCharacterID, int inNumber)
        {
            DeploymentUnitRecallButton tempButton = Instantiate(m_recallButtonPrefab);

            tempButton.transform.SetParent(gameObject.transform);
            tempButton.transform.localPosition = inPos;
            tempButton.transform.localScale = Vector3.one;
            tempButton.transform.SetAsLastSibling();

            tempButton.SetIdNumber(inCharacterID, inNumber);
            tempButton.m_actionRemove = (outCharacter, outNumber) =>
            {
                m_actionRecall?.Invoke(outCharacter, outNumber);

                DestroyRecallButton(outCharacter, outNumber);
            };

            m_listRecallButton.Add(tempButton);
        }

        public void DestroyRecallButton(int inCharacterID, int inNumber)
        {
            DeploymentUnitRecallButton temp = m_listRecallButton.Find(x => x.m_characterID == inCharacterID && x.m_number == inNumber);

            if( temp != null )
            {
                Destroy(temp.gameObject);
                m_listRecallButton.Remove(temp);
            }
        }
    }
}