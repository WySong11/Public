using Assets.InGame.Scripts.Deployment;
using Assets.InGame.UI.Scripts;
using Assets.InGame.UI.Scripts.Deployment;
using Definitions;
using PrinceCommon;
using PrinceNetwork;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UnityEngine.UIElements;
using UnityS.Mathematics;
using static Gpm.LogViewer.Internal.TabView;

namespace Assets.InGame.Lobby
{
    public class DeploymentPanel : MonoBehaviour
    {
        public DeploymentCharacterListWindow m_listWindow;
        public DeploymentInfoWindow m_infoWindow;
        public DeploymentTurnWindow m_turnWindow;        

        public GameHelper m_gameHelper;

        private TeamData m_leftTeamData = new TeamData(0);
        private TeamData m_rightTeamData = new TeamData(1);

        private int m_selectSetInId = 0;

        /// <summary>
        /// 현재 배치 진행 중인 팀 번호 0 : Left , 1 : Right
        /// </summary>
        private int m_teamIndex = 0;

        /// <summary>
        /// 현재 턴 진행 중인 진영. Left 라면 True
        /// </summary>
        private bool m_isLeft = false;

        public int eventType = 1;
        public float m_selectDistanceMin = 1;

        private bool m_isDragGround = false;

        /// <summary>
        /// 선택한 유닛 아이디.
        /// </summary>
        private int m_selectUnitIndex = -1;
        /// <summary>
        /// 선택된 유닛의 Array Index
        /// </summary>
        private int m_selectArrayIndex = -1;
        /// <summary>
        /// 배치된 Unit Number
        /// </summary>
        private int m_unitNumber = -1;

        /// <summary>
        /// 히든턴 사용했다면 True, 아니면 False
        /// </summary>
        public bool m_useHiddenTurn { get; private set; }

        private void OnEnable()
        {
            MEventHub.m_instance.Subscribe(OnGameLogicEvent, MEventType.UNIT_DEPLOYMENT);
            MEventHub.m_instance.Subscribe(OnGameLogicEvent, MEventType.TURN_END);
            MEventHub.m_instance.Subscribe(OnGameLogicEvent, MEventType.TURN_HIDDEN);

            switch (m_gameHelper.GetGameState())
            {
                case GameState.GameReady:
                    Reset();
                    m_gameHelper.NextState();
                    break;

                case GameState.LeftTurn:                    
                    Init(0, m_gameHelper.m_ruleData.GetTurnTime());
                    break;

                case GameState.RightTurn:
                    Init(1, m_gameHelper.m_ruleData.GetTurnTime());
                    break;

                default: break;
            }
        }

        private void OnDisable()
        {
            MEventHub.StopSubscribe(OnGameLogicEvent, MEventType.UNIT_DEPLOYMENT);
            MEventHub.StopSubscribe(OnGameLogicEvent, MEventType.TURN_END);
            MEventHub.StopSubscribe(OnGameLogicEvent, MEventType.TURN_HIDDEN);
        }

        private void OnGameLogicEvent(EventArg.EventArgInterface arg)
        {

            switch (arg.eventType)
            {
                case MEventType.UNIT_DEPLOYMENT:
                    {
                    }
                    break;

                case MEventType.TURN_END:
                    {
                        Debug.LogError("Event TURN_END");

                        var tempData = WebNetWorkManager.m_turnInfoList[WebNetWorkManager.m_turnInfoList.Count - 1];

                        if (tempData.Team != WebNetWorkManager.m_teamIndex)
                        {
                            CreateUnitList(tempData);
                        }

                        OnEndTurn(tempData.Team);
                    }
                    break;

                case MEventType.TURN_HIDDEN:
                    {
                    }
                    break;
            }
        }

        private void Update()
        {
            if (Input.GetKeyDown(KeyCode.Alpha1)) SelectUnit(1);
            if (Input.GetKeyDown(KeyCode.Alpha2)) SelectUnit(2);
            if (Input.GetKeyDown(KeyCode.Alpha3)) SelectUnit(3);
            if (Input.GetKeyDown(KeyCode.Alpha4)) SelectUnit(4);
        }

        private void Awake()
        {
            // 임시로 팀 고정
            m_leftTeamData.m_teamIndex = 0;
            m_rightTeamData.m_teamIndex = 1;

            m_gameHelper.m_onStateStart += OnGameStateStart;
            m_gameHelper.m_onStateEnd += OnGameStateEnd;

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// 유닛 배치하는 부분

            m_listWindow.m_actionBeginDrag = (outCharacter,outNumber, outPos) =>
            {
                SetInCharacter(GetCurrentTeamData(), outCharacter, outNumber, outPos, true, m_gameHelper.GetTurn());
                m_selectArrayIndex = GetCurrentTeamData().m_datas.Count - 1;
                m_selectUnitIndex = GetCurrentTeamData().m_datas[m_selectArrayIndex].m_id;
                m_unitNumber = outNumber;
            };

            m_listWindow.m_actionDragging = (outPos) =>
            {
                MoveInCharacter(GetCurrentTeamData(), m_selectArrayIndex, m_selectUnitIndex, m_unitNumber, outPos, m_gameHelper.GetTurn());
            };

            m_listWindow.m_actionEndDrop = (outPos, outActive) =>
            {
                if (CheckUnitCost() == false)
                {
                    RemoveCharacter(GetCurrentTeamData(), m_selectArrayIndex);
                }
                else if (outActive == true)
                {
                    if (MoveInCharacter(GetCurrentTeamData(), m_selectArrayIndex, m_selectUnitIndex, m_unitNumber, outPos, m_gameHelper.GetTurn()) == true)
                    {
                        m_listWindow.CreateRecallButton(m_selectUnitIndex, m_unitNumber);
                    }
                }
                else
                {
                    RemoveCharacter(GetCurrentTeamData(), m_selectArrayIndex);
                }

                RefreshUI();
            };

            m_listWindow.m_actionRecall = (outCharacter, outNumber) =>
            {
                int tempIndex = GetCurrentTeamData().m_datas.FindIndex(x => x.m_id == outCharacter && x.m_number == outNumber && x.m_turn == GameHelper.m_instance.GetTurn());

                if(tempIndex != -1)
                {
                    RemoveCharacter(GetCurrentTeamData(), tempIndex);
                    RefreshUI();
                }
                else
                {
                    Debug.LogErrorFormat("Error ActionRecall -> ID : {0} , Number : {1}", outCharacter, outNumber);
                }                
            };            

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// 유닛 옮기는 부분

            m_listWindow.m_actionBeginMove = (outPos) =>
            {
                if( CheckSetInCharacter(outPos, m_gameHelper.GetTurn()) == true)
                {
                    m_listWindow.DestroyRecallButton(m_selectUnitIndex, m_unitNumber);
                }
            };

            m_listWindow.m_actionMoving = (outPos, outArena, outScroll) =>
            {
                MoveInCharacter(GetCurrentTeamData(), m_selectArrayIndex, m_selectUnitIndex, m_unitNumber, outPos, m_gameHelper.GetTurn());
            };

            m_listWindow.m_actionEndMove = (outPos, outIndex, outActive) =>
            {
                if (outActive == true)
                {
                    if (MoveInCharacter(GetCurrentTeamData(), m_selectArrayIndex, m_selectUnitIndex, m_unitNumber, outPos, m_gameHelper.GetTurn()) == true)
                    {
                        m_listWindow.CreateRecallButton(m_selectUnitIndex, m_unitNumber);
                    }
                }
                else
                {
                    RemoveCharacter(GetCurrentTeamData(), m_selectArrayIndex);
                }

                RefreshUI();
            };

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            m_turnWindow.m_actionTurnEnd = () =>
            {
                if(WebNetWorkManager.m_isLogin == true)
                {
                    StartCoroutine(SendTurnEnd());
                }
                else
                {
                    m_gameHelper.NextState();
                }
            };

            m_turnWindow.m_actionHidden = () =>
            {
                if (WebNetWorkManager.m_isLogin == true)
                {
                    StartCoroutine(SendHiddenTurnEnd());
                }
                else
                {
                    m_useHiddenTurn = true;

                    TeamData tempData = GetCurrentTeamData();

                    tempData.m_hiddenTurnList.Add(m_gameHelper.GetTurn());

                    m_gameHelper.NextState();
                }
            };

            m_turnWindow.m_actionPrevious = () =>
            {
                //m_gameHelper.PreviousState();
            };
        }

        private void OnGameStateStart(GameState state)
        {

        }

        private void OnGameStateEnd(GameState state)
        {
            switch (state)
            {
                case GameState.LeftTurn:
                case GameState.RightTurn:
                    {
                        FinishSetIn();
                    }
                    break;
            }
        }

        public void Reset()
        {
            Debug.Log("DeploymentPanel Reset~!!!");

            m_listWindow.SetVisible(false);
            m_turnWindow.SetVisible(false);
            m_infoWindow.SetVisible(false);

            for (int i = 0; i < m_leftTeamData.m_datas.Count; ++i) RemoveUnits(m_leftTeamData, i);
            m_leftTeamData = new TeamData(0);

            for (int i = 0; i < m_rightTeamData.m_datas.Count; ++i) RemoveUnits(m_rightTeamData, i);
            m_rightTeamData = new TeamData(1);
        }

        public void Init(int teamIndex, int turnTime)
        {
            m_isLeft = teamIndex == 0;
            m_teamIndex = teamIndex;
            m_selectUnitIndex = -1;

            bool isMyTurn = (teamIndex == 0);

            if(WebNetWorkManager.m_isLogin == true)
            {
                isMyTurn = (teamIndex == WebNetWorkManager.m_teamIndex);

                TeamData tempTeamData = GetMyTeamData();

                m_listWindow.SetVisible(true, WebNetWorkManager.m_teamIndex == 0);
                m_listWindow.SortCharacterList(tempTeamData, m_gameHelper.GetTurnCost(WebNetWorkManager.IsLeftTeam()), GetCurrentTeamData().GetTurnCost(m_gameHelper.GetTurn()));

                m_turnWindow.SetTurnTime(turnTime);
                m_turnWindow.SetVisible(true, isMyTurn, tempTeamData.m_hiddenTurnList.Count, m_gameHelper.GetHiddenTurn());

                if (m_gameHelper.GetTurn() == m_gameHelper.GetTurnCount() && isMyTurn == false)
                {
                    m_turnWindow.ShowMessageBanner("상대가 마지막 턴을 배치 중 입니다.");
                }
                else if (m_useHiddenTurn == true && isMyTurn == true)
                {
                    m_turnWindow.ShowMessageBanner("상대가 히든 턴을 사용했습니다.");
                    m_useHiddenTurn = false;
                }

                m_infoWindow.SetVisible(true, true);
                m_infoWindow.DrawTurn(m_gameHelper.GetTurn(), m_isLeft);

                WebNetWorkManager.m_actionMatchDualTurn = OnMatchDualTurn;
            }
            else
            {
                TeamData tempTeamData = GetCurrentTeamData();
                tempTeamData.m_turn = m_gameHelper.GetTurn();

                m_listWindow.SetVisible(true, m_teamIndex == 0);
                m_listWindow.SortCharacterList(tempTeamData, m_gameHelper.GetTurnCost(IsLeft()), GetCurrentTeamData().GetTurnCost(m_gameHelper.GetTurn()));

                m_turnWindow.SetTurnTime(turnTime);
                m_turnWindow.SetVisible(true, isMyTurn, tempTeamData.m_hiddenTurnList.Count, m_gameHelper.GetHiddenTurn());

                if (m_useHiddenTurn == true)
                {
                    m_turnWindow.ShowMessageBanner("상대가 히든 턴을 사용했습니다.");
                    m_useHiddenTurn = false;
                }

                m_infoWindow.SetVisible(true, m_gameHelper.GetGameState() == GameState.LeftTurn);
                m_infoWindow.DrawTurn(m_gameHelper.GetTurn(), m_teamIndex == 0);
            }
        }

        public void ResetTeamData(TeamData inData, int inTeamIndex)
        {
            if (inData != null)
            {
                for (int i = 0; i < inData.m_datas.Count; ++i) RemoveUnits(inData, i);

                inData = new TeamData(inTeamIndex);
            }
        }

        public bool SetInCharacter(TeamData inTeam, int inCharacter, int inNumber, Vector2 inPos, bool inActive, int inTurn)
        {
            bool isLeft = inTeam.m_teamIndex == 0 ? true : false;

            // 표시 해야 할 영역인지 확인
            if (inActive == false) return false;

            if (m_gameHelper.GetUIPointToGamePoint(inPos, out Vector2 position) == false) return false;

            if (m_gameHelper.GetSetInPosition(inPos, out Vector2 result) == true)
            {
                SetIn(inTeam, inCharacter, inNumber, result, inTurn);

                return true;
            }

            return false;
            ///////////////////////////////////////////////////////////////////////////////////////////////
            /// 겹치는 캐릭터 지움
            //int selectIndex = -1;
            //float minDistance = float.MaxValue;
            //for (int i = 0; i < inTeam.m_datas.Count; ++i)
            //{
            //    float distance = Vector2.Distance(position, inTeam.m_datas[i].m_center_position);
            //    if (distance < minDistance)
            //    {
            //        selectIndex = i;
            //        minDistance = distance;
            //    }
            //}

            //if (minDistance > m_selectDistanceMin)
            //{
            //    if (GetSetInPosition(inPos, out Vector2 result) == true)
            //    {
            //        SetIn(inTeam, inCharacter, result);
            //    }
            //}
            //else
            //{
            //    RemoveUnits(inTeam, selectIndex);
            //    inTeam.SetOut(selectIndex);
            //}
            ///////////////////////////////////////////////////////////////////////////////////////////////
        }

        public bool CheckSetInCharacter(Vector2 inPos, int inTurn)
        {
            if (m_gameHelper.GetUIPointToGamePoint(inPos, out Vector2 position) == false) return false;

            var tempTeam = GetCurrentTeamData();

            for (int i = 0; i < tempTeam.m_datas.Count; ++i)
            {
                var data = tempTeam.m_datas[i];

                float distance = Vector2.Distance(position, data.m_center_position);
                if (distance < m_selectDistanceMin)
                {
                    // 턴이 다르면 불가능
                    if (data.m_turn != inTurn)
                    {
                        m_turnWindow.ShowMessageBanner("이전 턴에 배치한 캐릭터는 옮길 수 없습니다.");
                        m_listWindow.ResetDrag();
                        return false;
                    }

                    m_selectUnitIndex = tempTeam.m_datas[i].m_id;
                    m_unitNumber = tempTeam.m_datas[i].m_number;
                    m_listWindow.SetMoveCharacter(true, m_selectUnitIndex, inPos);

                    MoveInCharacter(GetCurrentTeamData(), i, m_selectUnitIndex, m_unitNumber, inPos, m_gameHelper.GetTurn());
                    m_selectArrayIndex = tempTeam.m_datas.Count - 1;
                    return true;
                }
            }

            return false;
        }

        public void Save()
        {
            TeamData tempData = GetCurrentTeamData();

            tempData.m_saveTime = DateTime.Now.ToString();
            string fileName = $"{DateTime.Parse(tempData.m_saveTime).ToString("yyyyMMddHHmmss")}_{tempData.m_teamIndex}_{tempData.m_datas.Count}.json";
            MUtils.Save("SetIn/", fileName, JsonUtility.ToJson(tempData));
            DialogWindow.m_instance.Show("알림", $"{fileName}\n저장되었습니다.", "common_ok".Localized("확인"), () => { });
        }

        private void FinishSetIn()
        {
            BattleManager.m_instance.TeamSetIn(GetCurrentTeamData());
        }

        private void RemoveUnits(TeamData inTeam, int index)
        {
            GameObject[] objs = inTeam.GetUnits(index);
            if (objs != null)
            {
                for (int i = 0; i < objs.Length; ++i) DestroyImmediate(objs[i]);
            }
        }

        public void LoadTeamData(TeamData data)
        {
            //! 기존 생성된 유닛 삭제
            if (data != null)
            {
                for (int i = 0; i < data.m_datas.Count; ++i) RemoveUnits(data, i);
            }

            for (int i = 0; i < data.m_datas.Count; ++i)
            {
                data.AttachUnit(i, GetUnits(data.m_teamIndex, data.m_datas[i]));
            }
        }

        private GameObject[] GetUnits(int teamIndex, CharacterSetInData data)
        {
            int unitCount = data.m_unitIds.Length;
            GameObject[] objs = new GameObject[unitCount];
            for (int i = 0; i < unitCount; ++i)
            {
                float2 tempPos = (float2)data.m_center_position + (float2)data.m_positions[i];
                objs[i] = CharacterFactory.GetUnitGameObject(teamIndex, data.m_unitIds[i], tempPos);
            }

            return objs;
        }

        private GameObject GetUnit(int teamIndex, int unitId, Vector2 position)
        {
            GameObject unit = CharacterFactory.GetUnitGameObject(teamIndex, unitId, (float2)position);
            return unit;
        }

        public string GetTime()
        {
            return DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
        }

        public void Load()
        {
            string[] files;
            MUtils.LoadFileList("SetIn/", out files);
            if (files.Length > 0) StartCoroutine(TeamDataLoad(m_teamIndex, files));
            else DialogWindow.m_instance.Show("배치 불러오기", "저장된 파일이 없습니다.", "common_ok".Localized("확인"), () => { });
        }

        IEnumerator TeamDataLoad(int teamIndex, string[] files)
        {
            List<TeamData> datas = new List<TeamData>();
            for (int i = 0; i < files.Length; ++i)
            {
                yield return null;
                string[] path = files[i].Split('/');
                string fileName = path[path.Length - 1];
                if (fileName.EndsWith(".meta") == true) continue;
                string json;
                bool success = MUtils.Load("SetIn/", fileName, out json);
                TeamData data = Load(json);
                if (data.m_teamIndex == teamIndex) datas.Add(Load(json));
            }
            yield return null;

            if (datas.Count == 0)
            {
                DialogWindow.m_instance.Show("popup_title_notice".Localized("알림"), "데이터가 없습니다.", LanguageManager.m_instance.GetText("common_yes"), null);
                yield break;
            }


            List<int> indexs = new List<int>();
            List<string> displays = new List<string>();

            for (int i = 0; i < datas.Count; ++i)
            {
                TeamData data = datas[i];
                displays.Add($"{data.m_teamIndex}팀 {data.m_datas.Count}개 배치 ({DateTime.Parse(data.m_saveTime)})");
                indexs.Add(i);
            }

            SelectListWindow.m_instance.SetData("리플레이", indexs, displays, (index) =>
            {
                SelectListWindow.m_instance.SetVisible(false);

                TeamData data = datas[index];
                LoadTeamData(data);

            }, null);
        }

        private TeamData Load(string json)
        {
            return JsonUtility.FromJson<TeamData>(json);
        }

        //public float m_dragScale = 0.0001f;

        //public void GroundDrag(BaseEventData data)
        //{
        //    PointerEventData pdata = data as PointerEventData;
        //    m_camera.transform.position += (Vector3)pdata.delta * m_dragScale;
        //}

        public void GroundDragStart(BaseEventData data)
        {
            m_isDragGround = true;
        }

        public void GroundDragEnd(BaseEventData data)
        {
            m_isDragGround = false;
        }

        public void SetIn(TeamData inTeam, int setInId, int inNumber, Vector2 position, int inturn)
        {
            UnitSetInRawData data = CharacterFactory.m_instance.GetUnitSetInData(setInId);

            SelectUnit(setInId);

            CharacterSetInData characterSetInData = new CharacterSetInData();
            characterSetInData.m_id = setInId;
            characterSetInData.m_turn = inturn;
            characterSetInData.m_number = inNumber;
            characterSetInData.m_center_position = position;
            characterSetInData.m_unitIds = (int[])data.m_ids.Clone();

            //! 팀간에 배치 포지션 y축 반전으로 수정 (상훈)
            characterSetInData.m_positions = data.GetUnitPositions(m_teamIndex);

            inTeam.SetIn(characterSetInData);
            inTeam.AttachUnit(inTeam.m_datas.Count - 1, GetUnits(m_teamIndex, characterSetInData));
        }

        public void SelectUnit(int id)
        {
            if (m_selectSetInId == id) return;
            m_selectSetInId = id;
            m_gameHelper.AddLog("SelectSetIn : " + m_selectSetInId);
        }

        public TeamData GetMyTeamData()
        {
            return WebNetWorkManager.m_teamIndex == 0 ? m_leftTeamData : m_rightTeamData;
        }

        public TeamData GetCurrentTeamData()
        {
            return m_teamIndex == 0 ? m_leftTeamData : m_rightTeamData;
        }

        public bool MoveInCharacter(TeamData inTeam, int arrayyindex, int setInId, int inNumber, Vector2 position, int inTurn)
        {
            RemoveCharacter(inTeam, arrayyindex);
            return SetInCharacter(inTeam, setInId, inNumber, position, true, inTurn);
        }

        public void RemoveCharacter(TeamData inTeam, int arrayIndex)
        {
            RemoveUnits(inTeam, arrayIndex);
            inTeam.SetOut(arrayIndex);
        }

        public bool IsLeft()
        {
            return GetCurrentTeamData().m_teamIndex == 0;
        }

        public void RefreshUI()
        {
            int teamcost = GetCurrentTeamData().GetTurnCost(m_gameHelper.GetTurn());

            m_infoWindow.UpdateCost(m_gameHelper.GetTurnCost(IsLeft()), teamcost);

            // 나중에 사용할수도 있음..
            //m_listWindow.SortCharacterList(GetCurrentTeamData(), m_gameHelper.GetTurnCost(IsLeft()), teamcost);

            m_listWindow.RefreshList();
        }

        public bool CheckUnitCost()
        {
            int teamcost = GetCurrentTeamData().GetTurnCost(m_gameHelper.GetTurn());

            if (teamcost > m_gameHelper.GetTurnCost(IsLeft()))
            {
                m_turnWindow.ShowMessageBanner("사용 가능한 코스트를 초과하였습니다.");
                return false;
            }

            return true;
        }

        IEnumerator SendTurnEnd()
        {
            bool isPacket = false;
            ServerPacketMatchDualTurn outPacket = new ServerPacketMatchDualTurn();

            WebNetWorkManager.SendMtachDualTurn(m_teamIndex, m_gameHelper.GetTurn(), m_gameHelper.GetTurn() == m_gameHelper.GetTurnCount(), false, GetUnitList(), (packet) =>
            {
                isPacket = true;
                outPacket = packet; 
            });

            yield return new WaitUntil(() => isPacket == true);

            OnEndTurn(outPacket.Team);

            //if (outPacket.Result == ePacketResult.OK)
            //{
            //    if(outPacket.Team != WebNetWorkManager.m_teamIndex)
            //    {
            //        CreateUnitList(outPacket);
            //    }

            //    // 혼자 할 때는, 제거 후 다시 배치.. 추후에 변경..
            //    //if (outPacket.UserNo != WebNetWorkManager.m_enemyUserInfo.m_userNo)
            //    //{
            //    //    DestroyUnitList(outPacket);

            //    //    CreateUnitList(outPacket);
            //    //}

            //    OnEndTurn(outPacket.Team);
            //}
        }

        IEnumerator SendHiddenTurnEnd()
        {
            bool isPacket = false;
            ServerPacketMatchDualTurn outPacket = new ServerPacketMatchDualTurn();

            WebNetWorkManager.SendMtachDualTurn(m_teamIndex, m_gameHelper.GetTurn(), m_gameHelper.GetTurn() == m_gameHelper.GetTurnCount(), true, GetUnitList(), (packet) =>
            {
                isPacket = true;
                outPacket = packet;
            });

            yield return new WaitUntil(() => isPacket == true);

            OnEndTurn(outPacket.Team, WebNetWorkManager.m_teamIndex == outPacket.Team);

            //if (outPacket.Result == ePacketResult.OK)
            //{
            //    // 혼자 할 때는, 안 보이게.. 추후에 변경..
            //    //if (outPacket.UserNo != WebNetWorkManager.m_enemyUserInfo.m_userNo)
            //    //{
            //    //    DestroyUnitList(outPacket);
            //    //}

            //    OnEndTurn(outPacket.Team, WebNetWorkManager.m_teamIndex == outPacket.Team);
            //}
        }

        public List<PacketUnit> GetUnitList()
        {
            TeamData tempTeam = GetCurrentTeamData();
            List<PacketUnit> tempList = new List<PacketUnit>();

            foreach(CharacterSetInData outData in tempTeam.m_datas)
            {
                if (outData.m_turn == m_gameHelper.GetTurn())
                {
                    PacketUnit tempUnit = new PacketUnit();

                    tempUnit.Index = outData.m_id;
                    tempUnit.Number = outData.m_number;
                    tempUnit.Center = new PacketVector(outData.m_center_position.x, outData.m_center_position.y);

                    tempUnit.Positions = new PacketVector[outData.m_positions.Length];

                    for (int i=0; i<outData.m_positions.Length; i++)
                    {
                        tempUnit.Positions[i] = new PacketVector(outData.m_positions[i].x, outData.m_positions[i].y);
                    }

                    tempList.Add(tempUnit);
                }
            }

            return tempList;
        }

        public void CreateUnitList(ServerPacketMatchDualTurn inPacket)
        {
            TeamData tempTeam = (inPacket.Team == 0) ? m_leftTeamData : m_rightTeamData;

            foreach (PacketUnit tempUnit in inPacket.UnitList)
            {
                UnitSetInRawData data = CharacterFactory.m_instance.GetUnitSetInData(tempUnit.Index);

                CharacterSetInData characterSetInData = new CharacterSetInData();
                characterSetInData.m_id = tempUnit.Index;
                characterSetInData.m_turn = inPacket.Turn;
                characterSetInData.m_number = tempUnit.Number;
                characterSetInData.m_center_position = new Vector2(tempUnit.Center.x, tempUnit.Center.y);
                characterSetInData.m_unitIds = (int[])data.m_ids.Clone();

                //! 팀간에 배치 포지션 y축 반전으로 수정 (상훈)
                characterSetInData.m_positions = data.GetUnitPositions(tempTeam.m_teamIndex);

                int unitCount = tempUnit.Positions.Length;
                GameObject[] objs = new GameObject[unitCount];
                for (int i = 0; i < unitCount; ++i)
                {
                    Vector2 tempV = new Vector2(tempUnit.Positions[i].x, tempUnit.Positions[i].y);
                    objs[i] = CharacterFactory.GetUnitGameObject(tempTeam.m_teamIndex, characterSetInData.m_unitIds[i], (float2)characterSetInData.m_center_position + (float2)tempV);
                }

                tempTeam.SetIn(characterSetInData);
                tempTeam.AttachUnit(tempTeam.m_datas.Count - 1, objs);
            }
        }

        public void DestroyUnitList(ServerPacketMatchDualTurn inPacket)
        {
            int arrayIndex = -1;

            TeamData tempTeam = (inPacket.Team == 0) ? m_leftTeamData : m_rightTeamData;

            foreach (PacketUnit tempUnit in inPacket.UnitList)
            {
                arrayIndex = tempTeam.m_datas.FindIndex(x => x.m_turn == inPacket.Turn && x.m_id == tempUnit.Index && x.m_number == tempUnit.Number);

                if(arrayIndex > -1)
                {
                    RemoveUnits(tempTeam, arrayIndex);
                    tempTeam.SetOut(arrayIndex);
                }
            }
        }

        public void  OnEndTurn(int inTeamIndex, bool inHiddenTurn=false)
        {
            if(inHiddenTurn == true)
            {
                m_useHiddenTurn = true;

                TeamData tempData = inTeamIndex == 0 ? m_leftTeamData : m_rightTeamData;

                tempData.m_hiddenTurnList.Add(m_gameHelper.GetTurn());
            }

            // 두 팀 다 턴 종료
            if(m_leftTeamData.m_turn == m_gameHelper.GetTurnCount() && m_rightTeamData.m_turn == m_gameHelper.GetTurnCount())
            {
                List<ServerPacketMatchDualTurn> tempList = WebNetWorkManager.m_turnInfoList.Where(x => x.Hidden == true).ToList();

                // 히든턴 유닛 배치
                foreach (ServerPacketMatchDualTurn tempInfo in tempList)
                {
                    Debug.LogFormat("Add Hidden Unit -> Team : {0} , Turn : {1} , UserNo : {2}", tempInfo.Team, tempInfo.Turn, tempInfo.UserNo);

                    CreateUnitList(tempInfo);
                }
            }

            m_gameHelper.NextState();
        }

        public void OnMatchDualTurn(ServerPacketMatchDualTurn packet)
        {
            Logger.LogError("OnMatchDualTurn~!!!!");

            //OnEndTurn(packet.Team, packet.Hidden);
        }
    }
}