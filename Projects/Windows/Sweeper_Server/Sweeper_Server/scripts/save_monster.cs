using System.IO;
using UnityEngine;

public class MushroomPositionRotationLogger : MonoBehaviour
{
    void Start()
    {
        // 파일 저장 경로 설정
        string path = "Assets/MushroomData.txt";

        // 파일 스트림 생성
        StreamWriter writer = new StreamWriter(path, true);

        // "Mushroom" 태그가 지정된 모든 게임 오브젝트를 검색하여 위치 및 회전 정보 기록
        foreach (GameObject obj in GameObject.FindGameObjectsWithTag("Mushroom"))
        {
            // 위치 정보 추출
            Vector3 position = obj.transform.position;
            // 회전 정보 추출 (Quaternion을 Euler 각도로 변환)
            Vector3 rotation = obj.transform.rotation.eulerAngles;

            // 파일에 위치 및 회전 정보 쓰기
            writer.WriteLine($"{position}{rotation}");
        }

        // 파일 닫기
        writer.Close();

        // 파일 저장 완료 메시지
        Debug.Log("Mushroom data logged to " + path);
    }
}
