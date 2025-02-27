Texture2D<float> ShadowMap : register(t0);
Texture2D<float4> IDTexture : register(t1); // 影のIDを持っているテクスチャ
RWBuffer<uint> EdgeBuffer : register(u0); // 輪郭を格納するバッファ 配列
RWBuffer<uint> CounterBuffer : register(u1); // 輪郭の数を格納するバッファ
static const uint kMaxIDNum = 32;
cbuffer Constants : register(b0)
{
    float2 dimensions; // テクスチャのサイズ
    float threshold; // しきい値
    uint idCount;
    int ID[kMaxIDNum]; // 輪郭のID
};
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // テクスチャの範囲外のスレッドは処理しない
    if (DTid.x >= dimensions.x || DTid.y >= dimensions.y)
        return;

    // テクスチャの座標を一次元に変換
    uint index = DTid.y * dimensions.x + DTid.x;

    // ピクセルが影か判定
    // 閾値より小さい場合は影
    bool isShadow = ShadowMap[DTid.xy] < threshold;

    // 影のとき
    bool isEdge = false;
    // ピクセル中心に周囲8ピクセルが影でないか調べる
    if(isShadow)
    {
        for (int y = -1; y <= 1; y++)
        {
                // 範囲外はスキップ
            if (DTid.y + y < 0 || DTid.y + y >= dimensions.y)
                continue;

            for (int x = -1; x <= 1; x++)
            {
                // 範囲外はスキップ
                if (DTid.x + x < 0 || DTid.x + x >= dimensions.x)
                    continue;

                // 閾値より大きい値があれば輪郭
                if (ShadowMap[DTid.xy + uint2(x, y)] >= threshold)
                {
                    isEdge = true;
                    break;
                }

            }
            if (isEdge)
            {
                break;
            }

        }

        // 輪郭のとき
        if(isEdge)
        {
            // カウンターをインクリメント
            uint counter;
            InterlockedAdd(CounterBuffer[0], 1, counter);
            EdgeBuffer[counter] = index;
        }

    }


}