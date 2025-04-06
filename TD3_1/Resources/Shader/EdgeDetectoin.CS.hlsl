Texture2D<float> ShadowMap : register(t0);
Texture2D<float4> IDTexture : register(t1); // 影のIDを持っているテクスチャ
RWBuffer<uint> EdgeBuffer : register(u0); // 輪郭を格納するバッファ 配列
RWBuffer<uint> CounterBuffer : register(u1); // 輪郭の数を格納するバッファ

cbuffer Constants : register(b0)
{
    float2 dimensions; // テクスチャのサイズ
    float threshold; // しきい値
    uint frameCount;
};

static const uint kMaxIDNum = 16;
cbuffer gID : register(b1)
{
    int4 ID[kMaxIDNum/4];
}

uint RestoreIDFromColor(float4 color);



[numthreads(1, 1, 1)]
void InitializeCounter(uint3 DTid : SV_DispatchThreadID)
{
    CounterBuffer[0] = 0;
    for (uint i = 0; i < 16; i++)
    {
        CounterBuffer[i] = 0;
    }
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // テクスチャの範囲外のスレッドは処理しない
    if (DTid.x >= dimensions.x || DTid.y >= dimensions.y)
        return;


    uint index = DTid.y * dimensions.x + DTid.x;
    // ピクセルが影か判定
    // 閾値より小さい場合は影
    bool isShadow = ShadowMap[DTid.xy] < threshold;
    uint id = RestoreIDFromColor(IDTexture[DTid.xy]);
    bool sameId = false;

    // 指定されたIDと一致するか確認
    for (uint i = 0; i < kMaxIDNum; i++)
    {
        if (id == ID[i / 4][i % 4])
        {
            sameId = true;
            break;
        }
    }

    // 影のとき
    bool isEdge = false;
    // ピクセル中心に周囲8ピクセルが影でないか調べる
    if (isShadow && sameId)
    {
        InterlockedAdd(CounterBuffer[10], 1);
        for (int y = -1; y <= 1; y++)
        {
            if (DTid.y + y < 0 || DTid.y + y >= dimensions.y)
                isEdge = true;

            for (int x = -1; x <= 1; x++)
            {
                if (DTid.x + x < 0 || DTid.x + x >= dimensions.x)
                    isEdge = true;

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
        if (isEdge)
        {
        // カウンターをインクリメント
            uint counter;
            InterlockedAdd(CounterBuffer[0], 1, counter);
            EdgeBuffer[counter] = index;
        }
    }

}
uint RestoreIDFromColor(float4 color)
{
    // RGB値から元のIDを再構築
    uint r_int = (uint) (color.r * 255.0f);
    uint g_int = (uint) (color.g * 255.0f);
    uint b_int = (uint) (color.b * 255.0f);
    // 各色チャンネルを適切な位置にシフトして元のIDを復元
    uint id = r_int | (g_int << 8) | (b_int << 16);
    return id;
}