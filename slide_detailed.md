---
marp: true
theme: default
paginate: true
header: 'sincos_acc: 深入 ARM NEON 三角函數優化細節'
footer: '深度技術分享 - 2026/04/24'
style: |
  section {
    font-family: 'Inter', 'Source Han Sans TC', sans-serif;
    font-size: 24px;
  }
  h1 { color: #2c3e50; }
  h2 { color: #34495e; border-bottom: 2px solid #3498db; padding-bottom: 5px; }
  code { background: #f8f9fa; color: #e83e8c; padding: 2px 4px; border-radius: 4px; }
  .grid-2 {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 20px;
  }
  .highlight-box {
    background: #e7f3ff;
    padding: 15px;
    border-left: 5px solid #3498db;
    margin: 10px 0;
  }
  svg { display: block; margin: auto; }
---

# 深入 ARM NEON 三角函數優化：從數學原理到微架構實作
### 揭開 4x 加速背後的數值計算與硬體調優技巧

**報告人：Gemini CLI**
**專案：sincos_acc**

---

## 1. 演算法核心：Minimax 逼近 vs. 泰勒級數

在數值計算中，我們並非追求「理論上的無窮精確」，而是在「計算代價」與「目標精確度」之間尋找最佳平衡。

<div class="grid-2">
<div>

### 泰勒級數 (Taylor Series)
- **原理**：在 $x_0$ 點進行無窮階展開。
- **缺點**：誤差隨 $|x-x_0|$ 增加而迅速發散（如左圖紅線）。
- **代價**：為了讓全區間精準，需要極高階的多項式。

</div>
<div>

### Minimax 近似 (Remez Algorithm)
- **原理**：尋找一個多項式 $P(x)$，使得在給定區間內 $\max |f(x) - P(x)|$ 最小。
- **特點**：誤差呈 **等波紋 (Equiripple)** 分佈（如左圖綠線），精度分佈極其均勻。

</div>
</div>

<svg width="600" height="200" viewBox="0 0 600 200">
  <path d="M 50 150 Q 150 150 300 150 T 550 150" stroke="#ccc" fill="transparent" />
  <!-- Taylor Error -->
  <path d="M 300 150 C 350 150 450 100 550 20" stroke="red" stroke-width="2" fill="transparent" />
  <!-- Minimax Error -->
  <path d="M 50 140 Q 100 160 150 140 Q 200 160 250 140 Q 300 160 350 140 Q 400 160 450 140 Q 500 160 550 140" stroke="green" stroke-width="2" fill="transparent" />
  <text x="560" y="30" font-size="12" fill="red">Taylor Error</text>
  <text x="560" y="145" font-size="12" fill="green">Minimax Error</text>
  <text x="300" y="170" font-size="14" text-anchor="middle">近似區間 [0, PI/4]</text>
</svg>

---

## 2. 精度守護者：Cody-Waite 值域縮減 (Deep Dive)

當 $x$ 很大時，計算 $x' = x - N \cdot \frac{\pi}{2}$ 會因為浮點數減法導致 **精度損失 (Precision Loss)**。

### 為什麼普通減法會失敗？
浮點數在表示 $\pi$ 時本身已有微小誤差。當 $x$ 很大，這個誤差會被放大 $N$ 倍。最終相減後，剩餘的有效數字位數可能不足 5 bits。

### Cody-Waite 解決方案：多段精確減法
我們將常數 $C \approx \frac{\pi}{4}$ 分解為：$C = C_1 + C_2 + C_3$
1. **$C_1$**：選取前 10-15 位，確保 $N \cdot C_1$ 在浮點數運算中 **不產生任何截斷誤差**。
2. **$C_2, C_3$**：儲存其餘的精確尾數。

<div class="highlight-box">
計算流程：
1. <code>x = x - N * C1</code> (精確無損)
2. <code>x = x - N * C2</code> (處理第一層殘差)
3. <code>x = x - N * C3</code> (處理極微小誤差)
</div>

---

## 3. NEON 微架構調優：無分支設計 (Branch-less)

在 SIMD 運算中，傳統的 `if-else` 是性能殺手。我們使用 **遮罩運算 (Masking)** 取代跳轉。

### 象限判定 (Quadrant Selection)
我們同時計算 $P_{sin}(x)$ 與 $P_{cos}(x)$，再根據象限位元 (Quadrant Bits) 進行選擇：

<svg width="700" height="250" viewBox="0 0 700 250">
  <!-- Register -->
  <rect x="50" y="50" width="400" height="40" fill="#f8f9fa" stroke="#333" />
  <text x="250" y="75" text-anchor="middle">Input X [x0, x1, x2, x3]</text>
  
  <!-- Logic -->
  <path d="M 250 90 L 150 140" stroke="#666" marker-end="url(#arrow)" />
  <path d="M 250 90 L 350 140" stroke="#666" marker-end="url(#arrow)" />
  
  <rect x="100" y="140" width="100" height="40" rx="5" fill="#d4edda" stroke="#28a745" />
  <text x="150" y="165" text-anchor="middle">Sin Poly</text>
  
  <rect x="300" y="140" width="100" height="40" rx="5" fill="#fff3cd" stroke="#ffc107" />
  <text x="350" y="165" text-anchor="middle">Cos Poly</text>
  
  <!-- Select -->
  <rect x="200" y="200" width="100" height="30" fill="#cce5ff" stroke="#004085" />
  <text x="250" y="220" text-anchor="middle" font-size="12">vbslq_f32 (Bitwise Select)</text>
  
  <defs>
    <marker id="arrow" markerWidth="10" markerHeight="10" refX="0" refY="3" orient="auto" markerUnits="strokeWidth">
      <path d="M0,0 L0,6 L9,3 z" fill="#666" />
    </marker>
  </defs>
</svg>

- **`vtstq_u32`**：檢查象限位元。
- **`vbslq_f32`**：根據遮罩，在不產生 CPU 分支預測的情況下，從兩個預算結果中挑選正確值。

---

## 4. Atan2 向量化：處理全平面座標

`atan2(y, x)` 的挑戰在於其 **不連續性** 與 **多象限跳轉**。

### 演算法流程：
1. **區間歸一化**：
   - 確保輸入落於 $[0, 1]$：若 $|y| > |x|$，則計算 $\text{atan}(x/y)$ 並補償 $\pi/2$。
2. **多項式求值**：
   - 使用高階多項式逼近 $\text{atan}(z)$。
3. **象限修正**：
   - **X < 0**：加上 $\pi$ 偏移。
   - **Y < 0**：最終結果取負。

### NEON 硬體對應：
- 使用 **`vabsq_f32`** 與 **`vcgtq_f32`** 建立交換遮罩。
- 使用 **`vnegq_f32`** 進行對稱性處理。
- **除法優化**：在 AArch64 上調用硬體 `vdivq_f32`，效能遠超查表法。

---

## 5. 最終實測：性能與精度矩陣

我們對所有函式進行了嚴格的單精度邊界測試。

| 函數 | 演算法細節 | 加速比 | 最大誤差 | 技術亮點 |
| :--- | :--- | :--- | :--- | :--- |
| **sincos** | Cody-Waite 3-step | **3.04x** | $7.5 \times 10^{-08}$ | 極高精度值域縮減 |
| **tan** | Rational / Newton | **2.96x** | $4.4 \times 10^{-04}$ | Sin/Cos 同步利用 |
| **asin** | LUT + Sqrt | **2.96x** | $4.8 \times 10^{-04}$ | 查表與多項式混合 |
| **atan2** | **Full SIMD Mapping** | **4.05x** | $1.1 \times 10^{-05}$ | 無分支全平面路徑 |

### 加速比分析
為什麼 `atan2` 提升最高？
> 因為 `math.h` 的 `atan2f` 涉及極其複雜的分支跳轉與邊界檢查；而 NEON 版本透過位元遮罩將所有路徑「攤平」為固定長度的指令流，徹底消除了分支預測失敗的開銷。

---

## 6. 結論與最佳實踐

1. **數據對齊 (Data Alignment)**：
   - 使用 `vld1q_f32` 載入數據。若數據未對齊 16-byte，建議使用 `vld1q_f32_x1` 或確保指標對齊。
2. **多項式係數選擇**：
   - 本專案使用的係數經過 Remez 演算法優化，直接硬編碼於指令中（Immediate Load），避免記憶體存取。
3. **未來的方向**：
   - 擴充對 `log10`, `pow`, `exp` 的完全向量化支持。
   - 引入對 AArch64 專有指令（如 `fmla`）的更深度利用。

---

# 感謝分享！
### 源碼與技術文件請參考：`include/sincos_neon.h`
