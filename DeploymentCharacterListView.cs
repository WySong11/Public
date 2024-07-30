using EnhancedUI.EnhancedScroller;
using System;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections.Generic;

namespace Assets.InGame.UI.Scripts.Deployment
{
    public delegate void SelectedViewDelegate(DeploymentCharacterListView cellView);

    public class DeploymentCharacterListView : MonoBehaviour, IBeginDragHandler, IDragHandler, IEndDragHandler, IPointerClickHandler
    {
        public enum State
        {
            None,
            Select,
            Drag,       // angle
            Scroll,     // Shot
        }
        private float m_clickTime = 0.0f;

        public UnitListData m_listData;
        public CharacterView m_view;
        public GameObject m_dimBg;
        public Button m_recallButton;

        public Action<int, int, Image, Vector2> m_actionBeginDrag;
        public Action<Vector2> m_actionDragging;
        public Action<Vector2> m_actionEndDrag;
        public Action<int> m_actionDoubleClick;
        public Action<int, int> m_actionRecall;

        [Header("= 스크롤 감도 조절. =")]
        public float m_angleDegree = 45;
        public float m_shotRange = 25;
        public int m_checkFrame = 1;
        private int m_checkCount = 0;

        private State m_state = State.None;
        private int m_pingerId;
        public bool m_isUsePingerCheck = true;

        public SelectedViewDelegate selected;

        public EnhancedScroller m_scroller { get; private set; }

        public Vector2 m_startPos { get; private set; }
        public bool m_isLeft { get; private set; }
        public bool m_viewContainerInScrollRect { get; private set; }
        public bool m_isViewContainer { get; private set; }

        public void Update()
        {

        }

        private RectTransform _rectTransform;
        public RectTransform m_rectTransform
        {
            get
            {
                if (_rectTransform == null)
                {
                    _rectTransform = GetComponent<RectTransform>();
                }

                return _rectTransform;
            }

            private set
            {
                _rectTransform = value;
            }
        }

        private void Awake()
        {
            m_recallButton.onClick.RemoveAllListeners();
            m_recallButton.onClick.AddListener(() =>
            {
                OnRecallButtonClick();
            });
        }

        public void SetData(UnitListData inData, TeamData inTeamData, int inTurnCost, int inUseCost)
        {
            if (inData != null)
            {
                m_listData = inData;
                m_view.SetData(m_listData.m_id, inData, inTeamData, inTurnCost, inUseCost);

                // 배치 중인 상태 찾기
                var tempData = inTeamData.m_datas.Find(x => x.m_id == inData.m_id && x.m_number == inData.m_number && x.m_turn == GameHelper.m_instance.GetTurn());

                //if(tempData != null)
                //{
                //    Debug.LogFormat("inData -> id : {0} , number : {1}", inData.m_id, inData.m_number);
                //}

                m_dimBg.SetActive(tempData != null);
                m_recallButton.gameObject.SetActive(tempData != null);

                gameObject.SetActive(true);
            }
            else
            {
                m_listData = null;
                gameObject.SetActive(false);
            }
        }

        public void SetScroller(EnhancedScroller inScroller, bool inisLeft, bool inViewContainerInScrollRect, float clickTime)
        {
            m_scroller = inScroller;
            m_isLeft = inisLeft;
            m_viewContainerInScrollRect = inViewContainerInScrollRect;
            m_clickTime = clickTime;
        }

        public void OnBeginDrag(PointerEventData eventData)
        {
            if (m_isUsePingerCheck == true)
            {
                if (m_state != State.None)
                {
                    Debug.Log("m_state != State.None ====> " + m_state);
                    return;
                }

                m_pingerId = eventData.pointerId;
            }

            m_pingerId = eventData.pointerId;

            m_checkCount = 0;

            ChangeState(State.Select);
        }

        public void OnDrag(PointerEventData eventData)
        {
            //// 스크롤 영역인지 확인
            //bool inScrollRect = false;

            //if (m_isLeft == true)
            //{
            //    inScrollRect = eventData.position.x <= m_scroller.GetScrollWidhHeight;
            //}
            //else
            //{
            //    inScrollRect = eventData.position.x >= Screen.width - m_scroller.GetScrollWidhHeight;
            //}

            //if (m_viewContainerInScrollRect == true)
            //{
            //    if (m_isDragging == true && m_isViewContainer == false/* && inScrollRect == true*/)
            //    {
            //        if (Mathf.Abs(m_startPos.x - eventData.position.x) > m_minDistance)
            //        {
            //            m_isViewContainer = true;
            //        }
            //        else if (Mathf.Abs(m_startPos.y - eventData.position.y) > m_minDistance)
            //        {
            //            m_isDragging = false;
            //            m_isViewContainer = false;
            //        }
            //    }
            //}
            //else
            //{
            //    m_isViewContainer = (inScrollRect == false);

            //    // 스크롤 영역 벗어났는지 확인
            //    if (inScrollRect == true && Mathf.Abs(m_startPos.y - eventData.position.y) > m_maxDistance)
            //    {
            //        m_isDragging = false;
            //    }
            //}

            //if (m_isDragging == true && m_dimBg.activeSelf == false)
            //{
            //    m_actionDragging?.Invoke(eventData.position, true);
            //}
            //else
            //{
            //    eventData.pointerDrag = m_scroller.ScrollRect.gameObject;
            //    EventSystem.current.SetSelectedGameObject(m_scroller.ScrollRect.gameObject);

            //    m_scroller.ScrollRect.OnInitializePotentialDrag(eventData);
            //    m_scroller.ScrollRect.OnBeginDrag(eventData);

            //    m_actionEndDrag?.Invoke(eventData.position, false);
            //}

            if (IsCheckPingerId(eventData.pointerId) == false) return;

            switch (m_state)
            {
                case State.None:
                    break;
                case State.Select:
                    {
                        m_checkCount++;
                        if (m_checkCount >= m_checkFrame)
                        {
                            Vector2 deltaPosition = eventData.position - eventData.pressPosition;
                            State state = GetState(deltaPosition);

                            if (m_dimBg.activeSelf == true)
                            {
                                state = State.Scroll;
                            }                            

                            switch (state)
                            {
                                case State.Drag:
                                    m_actionBeginDrag?.Invoke(m_listData.m_id, m_listData.m_number, m_view.m_characterImage, eventData.position);
                                    ChangeState(State.Drag);
                                    break;

                                case State.Scroll:
                                    eventData.pointerDrag = m_scroller.ScrollRect.gameObject;
                                    EventSystem.current.SetSelectedGameObject(m_scroller.ScrollRect.gameObject);
                                    m_scroller.ScrollRect.OnInitializePotentialDrag(eventData);
                                    m_scroller.ScrollRect.OnBeginDrag(eventData);
                                    ChangeState(State.None);
                                    break;

                                default:
                                    {
                                        Debug.Log("삑사리");
                                    }
                                    break;
                            }
                        }
                    }
                    break;
                case State.Drag:
                    {
                        m_actionDragging?.Invoke(eventData.position);
                    }
                    break;
                case State.Scroll:
                    {
                    }
                    break;
            }
        }

        public void OnEndDrag(PointerEventData eventData)
        {
            if (IsCheckPingerId(eventData.pointerId) == false)
            {
                Debug.Log("OnEndDrag IsCheckPingerId false");
                return;
            }

            //! 바로 드레그로 인식되지 않도록 처리
            m_pingerId = -1;

            switch (m_state)
            {
                case State.Drag:
                    {
                        m_actionEndDrag?.Invoke(eventData.position);
                    }
                    break;

                case State.Scroll:
                    {
                    }
                    break;

                default:
                    {
                    }
                    break;
            }

            ChangeState(State.None);
        }

        public void OnSelected()
        {
            // if a handler exists for this cell, then
            // call it.
            if (selected != null) selected(this);
        }

        public void OnPointerClick(PointerEventData eventData)
        {
            if((Time.time - m_clickTime) < 0.3f)
            {
                m_actionDoubleClick?.Invoke(m_listData.m_id);
                m_clickTime = -1;
            }
            else
            {
                m_clickTime = Time.time;
            }
        }

        public void OnRecallButtonClick()
        {
            m_actionRecall?.Invoke(m_listData.m_id, m_listData.m_number);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// 스크롤 관련 추가 부분
        /// 
        private bool IsCheckPingerId(int pingerId)
        {
            if (m_isUsePingerCheck == false) return true;

            return m_pingerId == pingerId;
        }

        private void ChangeState(State state)
        {
            m_state = state;
        }

        private State GetState(Vector2 delta)
        {
            float abs_x = Mathf.Abs(delta.x);
            float abs_y = Mathf.Abs(delta.y);

            if (abs_x == 0) return State.Scroll;
            if (abs_y == 0) return State.Drag;

            float angle = Vector2.Angle(Vector2.right, new Vector2(abs_x, abs_y));

            if (angle < m_angleDegree)
            {
                return State.Drag;
            }

            if (90 - angle < m_shotRange)
            {
                return State.Scroll;
            }

            return State.Select;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
    }
}