---
marp: true
theme: default
paginate: true
header: 'sincos_acc: ARM NEON 加速三角函數實作'
footer: '技術分享 - 2026/04/24'
style: |
  section {
    font-family: 'Inter', sans-serif;
  }
  code {
    background: #f4f4f4;
    color: #d63384;
  }
---
# 基於 ARM NEON 的高效能平行化 Sine/Cosine 演算法優化
### 從數學原理到 SIMD 硬體加速實作

---
## 1. 為什麼需要加速？ (Motivation)
- **性能瓶頸**：在嵌入式系統中，標準庫 `<math.h>` 的 `sinf()` / `cosf()` 是純純的 Scalar運算。
- **大量運算需求**：訊號處理 (DSP)、機器人運動學 (Inverse Kinematics)
      往往需要對整個數組進行三角運算。
- **SIMD 的優勢**：ARM NEON 提供 128-bit 暫存器，單次指令可處理 4 個
      `float`，理論吞吐量提升 4 倍。
---
## 2. 核心演算法：Cephes 數學庫
本專案基於經典的 **Cephes** 演算法進行向量化改造：
- **逼近策略**：不使用展開項極多的泰勒級數，而是使用 **Minimax 近似多項式**。
- **值域縮減 (Range Reduction)**：將輸入 $x$ 映射至 $[0, \pi/4]$ 區間。
- **對稱性利用**：透過整數運算與位元遮罩判定象限，決定最終結果的符號。
---
2. 核心演算法：多項式逼近與並行策略 (詳細)
2.1 為什麼選擇 Minimax 而非 Taylor？
* 泰勒級數 (Taylor Series)：在 $x=0$ 附近極其精準，但隨著遠離中心點，誤差會迅速發散。若要維持全區間精度，需要極高階的多項式，運算成本高。
* Minimax 逼近：
- * 透過 Remez 演算法 產生的係數，將「最大誤差」最小化。
- * 在目標區間 $[0, \pi/4]$ 內，誤差分佈均勻（Equiripple），能以較低的多項式階數達到同樣的單精度精準度。
- * 本專案實作：使用 6 階多項式即可滿足 float 的精確度需求。
---
2.2 霍納方法 (Horner's Method) 與 FMA
  為了減少乘法次數並利用硬體加速，我們將多項式 $a_n x^n + ... + a_0$ 寫成嵌套形式：
  $$y = ((a_6 \cdot z + a_4) \cdot z + a_2) \cdot z + ...$$
* NEON 優化：每一層嵌套都直接對應一條 vfmaq_f32 (Fused Multiply-Add) 指令。
* 效益：僅需一次指令週期即可完成「乘法 + 加法」，且減少了中間結果的捨入誤差。

---
2.3 並行象限處理 (Parallel Quadrant Logic)
傳統程式碼會用 if/else 判斷象限，但在 SIMD 中這會導致 Pipeline Stall。
* 核心策略：同時計算 sin_poly 與 cos_poly。
* Bitwise Selection：
- * 根據 $x$ 縮放後的整數部分，生成控制遮罩。
- * 使用 vtstq_u32 檢查位元，判斷是否需要交換 Sin/Cos 結果或翻轉正負號。
- * 最後透過 vbslq_f32 (Bitwise Select) 在不產生分支的情況下選出正確結果。
---

## 3. 精度關鍵：Cody-Waite 值域縮減
<!-- 當 $x$ 很大時，直接用 `x % pi` 會導致精確度喪失。我們採用 **Cody-Waite** 策略：
- **多段 $\pi$ 常數**：將 $\pi/4$ 分解為高精度、低誤差的三個部分（DP1, DP2, DP3）。
- **公式**：
$x_{reduced} = ((x - y \cdot DP1) - y \cdot DP2) - y \cdot DP3$
- **效益**：確保在 $x < 8192$ 時仍能維持單精度浮點數的極限精度。 -->
## 3.1 大角度下的「精度災難」
當輸入 $x$ 很大時（例如 $x=10000$），計算 $x \pmod{2\pi}$ 會面臨嚴重挑戰：
* 單精度浮點數 $\pi$ 的表示本身就有誤差。
* 當 $x$ 與 $\pi$ 相減時，高位數會抵消（Catastrophic Cancellation），導致結果的低位有效數字全是垃圾。
* 現象：傳統算法在 $x > 100$ 時精度就會開始崩壞。
---
## 3.2 Cody-Waite 演算法解決方案
我們將 $\pi/4$ 分解為三個部分：$DP1, DP2, DP3$。
* DP1：包含 $\pi/4$ 的高位數，且其尾端補零，確保 $y \cdot DP1$ 在浮點運算中不產生任何捨入誤差。
* DP2 & DP3：存儲剩餘的精確尾數。
* 計算流程：
       1. $x' = x - y \cdot DP1$ (精確計算)
       2. $x'' = x' - y \cdot DP2$
       3. $x_{final} = x'' - y \cdot DP3$
* 這種類似「雙倍精度模擬」的手法，讓單精度運算在巨大角度下依然能保持 $10^{-7}$ 級別的誤差。
---
## 3.3 常數設計細節 (Code Insight)
在 extern/neon_mathfun.h 中定義的常數：
1 #define c_minus_cephes_DP1 -0.78515625              // 11001010.1
2 #define c_minus_cephes_DP2 -2.4187564849853515625e-4
3 #define c_minus_cephes_DP3 -3.77489497744594108e-8
* 這些數值並非隨機，而是經過精心挑選，使得 $y \cdot DP1$ 的結果能夠精確地表達在 float 的23-bit 尾數中，不會發生截斷。
* 這是本專案在高頻震盪訊號處理中仍能保持穩定的核心祕訣。
---
## 4. ARM NEON 加速架構
我們實作了向量化的 `sincos_ps(float32x4_t x, ...)`：
```c++
// 核心平行化循環
for (i = 0; i < vec_count; i += 4) {
  // 1. 同時載入 4 個輸入
  float32x4_t v_in = vld1q_f32(&input[i]);
  // 2. SIMD 平行計算 sin 與 cos
  sincos_ps(v_in, &v_sin, &v_cos);
  // 3. 同時存入 4 個結果
  vst1q_f32(&sin_out[i], v_sin);
  vst1q_f32(&cos_out[i], v_cos);
}
```

---
## 5. NEON 優化技巧 Deep Dive
為了極致效能，我們避開了條件分支，改用硬體原語：
- **FMA (Fused Multiply-Add)**:
使用 `vfmaq_f32` 減少指令週期，同時提高多項式累加的精確度。
- **無分支邏輯 (Branch-less)**:
利用 `vbslq_f32` (Bitwise Select) 根據象限遮罩選擇結果，避免 CPU 分支預測錯誤 (Branch Misprediction)。
- **位元運算**:
使用 `veorq_u32` 快速切換符號位，取代傳統的 `if (x < 0) ...`。
---
## 6. 精度分析：實測數據
將 NEON 版與標準 C 庫 (`sinf`/`cosf`) 進行對比（$N=66,820$）：
| 指標 | 數值 |
| :--- | :--- |
| **最大絕對誤差 (Max Error)** | $7.52 \times 10^{-08}$ |
| **均方根誤差 (RMSE)** | $2.25 \times 10^{-08}$ |
| **測試結果** | **PASSED** (優於 $10^{-5}$ 目標) |
> 結論：在肉眼不可見的精度損失下，換取極大的效能提升。
---
## 7. 效能表現：3.05x 加速
在 ARM64 環境下的基準測試結果：
- **資料規模**：66,820 個元素
- **Scalar Throughput**：554.16 M/s
- **NEON Throughput**：**1689.57 M/s**
- **實測加速比**：**3.05 倍**
> 備註：受限於記憶體對齊與 Cody-Waite 複雜指令，雖未達 4x 理論值，但已大幅超越傳統寫法。
---
## 8. 開發者實作建議
1. **記憶體對齊 (Alignment)**：
確保 `float*` 對齊 16-byte，否則 `vld1q` 可能觸發效能懲罰。
2. **尾端處理 (Tail Handling)**：
處理 `count % 4 != 0` 的剩餘數據，使用傳統 `sinf` 補足。
3. **架構分歧**：
AArch64 支援 `vrndmq_f32` (Floor)，舊型 AArch32 需手動模擬。
---
## 9. 總結 (Summary)
- **高效能**：透過 NEON SIMD 實現 300% 的吞吐量增長。
- **高精確度**：採用 Cody-Waite 值域縮減與 Cephes 多項式。
- **無分支設計**：最大化處理器流水線 (Pipeline) 效率。
- **適用場景**：嵌入式視覺、即時感測器融合、高效能物理模擬。
