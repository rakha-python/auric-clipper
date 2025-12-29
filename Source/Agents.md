# AURIC Z-CLIP — DESIGN-ONLY AGENT BRIEF (Codex CLI)

## 0) Tujuan Utama (NON-NEGOTIABLE)

Bangun ulang UI plugin **AURIC Z-CLIP** mengikuti referensi gambar **1:1**:

* **Layout sama** (posisi tiap elemen, jarak, alignment, hierarchy).
* **Warna sama** (tint, brightness, glow, shadow).
* **Font sama/semirip mungkin** (kerning, tracking, casing, weight).
* **Kedalaman 3D realistis** (hardware look: brushed metal, bevel, rim, cast shadow, bloom, vignette).
* **Tidak fokus fitur/DSP dulu** — hanya UI/visual sampai screenshot output “indistinguishable” dari referensi.

## 1) Input Referensi

* Gunakan image referensi yang sudah tersedia di workspace project:

  * `image.png`
* Referensi adalah **single source of truth** untuk:

  * grid layout,
  * ukuran knob,
  * thickness ring,
  * intensitas glow,
  * tekstur metal,
  * ukuran teks dan tracking.

## 2) Output yang Wajib Dihasilkan (Deliverables)

1. UI plugin JUCE yang ketika dijalankan menghasilkan tampilan:

   * **proporsi sama** dengan referensi,
   * **komponen di tempat yang sama**,
   * **lighting & depth mendekati identik**.

2. File yang boleh diubah/ditambah (design layer saja):

* `Source/PluginEditor.h/.cpp` (layout + bounds)
* `Source/AuricLookAndFeel.h/.cpp` (seluruh style drawing)
* `Source/AuricKnob.h/.cpp` (big knob & small knob components)
* (Opsional) komponen baru:

  * `ScrewComponent.*`
  * `NeonRing.*`
  * `BottomGlowStrip.*`
  * `MetalPlateBackground.*`

3. Jangan ubah DSP / parameter logic (kecuali stub visual).

## 3) Acceptance Criteria (Wajib Lulus)

### A. Pixel/Geometry Matching

* Posisi tiap elemen harus match saat dibandingkan overlay dengan referensi:

  * 4 screw di corner,
  * tulisan “AURIC” kiri atas,
  * “Z-CLIP” kanan atas,
  * big knob center,
  * 4 small knob di kuadran kiri/kanan,
  * label “DRIVE” di bawah big knob,
  * toggle “2X” + slider output ceiling + LED pink + glow strip bawah.
* Target: **maks deviasi 1–2 px** pada ukuran 1x.

### B. Material & Depth

* Panel: brushed metal horizontal + vignette + inner rim.
* Knob: bezel + cavity + specular highlight realistis.
* Neon ring big knob: glow magenta dengan **bloom** dan **falloff** halus.
* Small knobs: ring tick marks + pointer tip + depth consistent.

### C. Typography

* Semua label uppercase dengan tracking sesuai referensi.
* Warna teks abu-abu pudar untuk label kecil; magenta untuk brand.
* Kontras tepat: tidak terlalu putih.

## 4) Metode Kerja (Wajib Ikuti)

### Step 1 — Kanvas & Grid

* Set ukuran editor mengikuti aspect ratio referensi (jangan “ngarang”).
* Definisikan grid anchor:

  * `panelBounds` = full window
  * `innerPlateBounds` = panelBounds reduced (rim thickness)
  * Titik referensi: center big knob, baseline teks atas, garis glow bawah.

### Step 2 — Background Plate (hardware)

Implement background terlebih dahulu:

* Brushed metal (noise halus + horizontal streak)
* Vignette (lebih gelap di tepi)
* Inner rim bevel (highlight atas-kiri, shadow bawah-kanan)
* Jangan over-contrast: referensi itu “soft”.

### Step 3 — Screws Corner

* 4 screw identik:

  * outer ring + inner slot/philips
  * subtle highlight, shadow, dan ambient occlusion di sekitar lubang.

### Step 4 — Typography Layer

* “AURIC” kiri atas: magenta, besar, sedikit glow halus.
* “Z-CLIP” kanan atas: magenta, ukuran mirip.
* Label knob (PRE, TRIM, SAT/CLIP, MIX): grey, tracking lebar, posisinya match.
* “DRIVE” besar bawah big knob: magenta, center.

### Step 5 — Knobs

#### Big Knob (CENTER)

Harus jadi “hero” dan match 100%:

* Outer bezel: dark metal ring + subtle rim highlight.
* Inner face: radial gradient + specular swirl tipis.
* Neon ring:

  * **double ring** (core bright + outer bloom)
  * glow spill ke panel (pinkish).
* Perhatikan: ring tidak boleh “kartun”; harus soft & realistic.

#### Small Knobs (4 pcs)

* Sama bentuk/lighting, hanya berbeda posisi.
* Ada tick marks halus di sekitar knob.
* Pointer putih/abu tipis di atas knob (posisi jam 12-ish di referensi).
* Depth: knob “masuk” sedikit ke panel dengan shadow halus.

### Step 6 — Lower Controls

* LED pink kiri bawah:

  * lens glass + hotspot + outer ring + ambient glow.
* “2X” label dekat toggle: grey.
* Slider Output Ceiling (horizontal) di bawah big knob:

  * track slot + handle kecil, metal/plastic dengan bevel.
* Bottom glow strip (magenta) sepanjang bawah panel:

  * bright core + soft blur upward.

## 5) Aturan Implementasi JUCE (Style)

* Semua drawing pakai float bounds, anti-alias on.
* Gunakan:

  * `g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);`
  * `juce::DropShadow` hanya untuk cast shadow lembut (jangan keras).
* Buat helper untuk:

  * `drawMetalPlate()`
  * `drawScrew()`
  * `drawNeonRing()`
  * `drawKnobFace()`
* **Tidak boleh** ada background “kotak aneh” di belakang knob.

  * Knob harus menyatu ke panel (AO + shadow natural).

## 6) Warna (Guideline — final harus di-sample dari referensi)

> Catatan: gunakan ini sebagai baseline, lalu **fine-tune** dengan sampling dari referensi sampai match.

* Panel base: #0B0C0D – #151618 (gradient halus)
* Text grey: #6E6E6E (label), #8A8A8A (lebih terang tipis)
* Magenta brand: sekitar #B03A7A – #D946EF (tergantung area)
* Neon core: #FF4FD8 (lebih terang), bloom: #E879F9 (lebih soft)

## 7) Font (Guideline)

* Pakai font geometric sans yang paling mendekati referensi.
* Jika repo sudah punya font custom: gunakan itu.
* Jika tidak ada:

  * fallback: `Inter`, `Montserrat`, atau `Sora`
  * set tracking manual untuk meniru style (tracking lebar untuk label kecil).
* Wajib uppercase, weight:

  * brand: semi-bold
  * label: medium/light

## 8) Scaling & Responsiveness

* UI harus tetap “seperti foto” pada resize:

  * scale uniform berdasarkan min(width,height),
  * semua posisi relatif terhadap anchor grid,
  * jangan berubah layout/hierarchy.

## 9) Cara Verifikasi (Wajib)

* Buat mode debug overlay:

  * load referensi image sebagai overlay semi-transparan (toggle via key).
  * compare knob center, teks baseline, dan screw alignment.
* Iterasi sampai perbedaan tidak terlihat.

## 10) Larangan (Hard NO)

* Jangan redesign kreatif.
* Jangan ganti layout.
* Jangan ubah style jadi flat.
* Jangan bikin glow berlebihan.
* Jangan prioritas DSP/parameter logic.

## 11) Definition of Done

UI screenshot output:

* Kalau ditempel berdampingan dengan referensi, user awam **tidak bisa bedakan**.
* Semua elemen terasa hardware 3D realistis, bukan “UI game”.

— END —
