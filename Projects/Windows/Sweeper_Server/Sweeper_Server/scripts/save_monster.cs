using System.IO;
using UnityEngine;

public class MonsterPositionRotationLogger : MonoBehaviour
{
    void Start()
    {
        // 파일 저장 경로 설정
        string path = "Assets/MonsterData.txt";
        string[] tags = { "Mushroom", "BornDog", "Goblin", "Boogie" };

        // 파일 스트림 생성
        using (StreamWriter writer = new StreamWriter(path, true))
        {
            for (int i = 0; i < tags.Length; i++)
            {
                // 해당 태그를 가진 모든 게임 오브젝트 검색
                foreach (GameObject obj in GameObject.FindGameObjectsWithTag(tags[i]))
                {
                    // 위치 정보 추출
                    Vector3 position = obj.transform.position;
                    // 회전 정보 추출 (Quaternion을 Euler 각도로 변환)
                    Vector3 rotation = obj.transform.rotation.eulerAngles;

                    // 파일에 위치 및 회전 정보 쓰기
                    writer.WriteLine($"({i}){position}{rotation}");
                }
            }
        }

        // 파일 저장 완료 메시지
        Debug.Log("Monster data logged to " + path);
    }
}
