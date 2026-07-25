// dev.mauicpp.MauiCenteredImageSpan — an android.text.style.ImageSpan that vertically CO-CENTERS the icon
// with the text, reproducing MAUI's MauiMaterialButton IconGravity=ICON_GRAVITY_TEXT_START, where the
// [icon | text] group shares ONE vertical centre. The stock ImageSpan(ALIGN_CENTER) leaves the text on its
// own baseline inside an icon-inflated line, so a tall button icon sits ABOVE the lower text (button page:
// the "settings" label rode below the gear). getSize grows the line SYMMETRICALLY about the font's vertical
// centre so the icon fits without shoving the text down, and draw places the icon so its centre matches the
// text's vertical centre — the two now share a centre-line, matching MAUI.
package dev.mauicpp;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.text.style.ImageSpan;

public class MauiCenteredImageSpan extends ImageSpan {
    public MauiCenteredImageSpan(Drawable drawable) {
        super(drawable, ImageSpan.ALIGN_CENTER);
    }

    @Override
    public int getSize(Paint paint, CharSequence text, int start, int end, Paint.FontMetricsInt fm) {
        Drawable d = getDrawable();
        Rect rect = d.getBounds();
        if (fm != null) {
            Paint.FontMetricsInt pfm = paint.getFontMetricsInt();
            int fontCentre = (pfm.ascent + pfm.descent) / 2; // ascent is negative, so this is the vertical centre
            int half = rect.height() / 2;
            fm.ascent = Math.min(pfm.ascent, fontCentre - half);
            fm.top = fm.ascent;
            fm.descent = Math.max(pfm.descent, fontCentre + half);
            fm.bottom = fm.descent;
        }
        return rect.width();
    }

    @Override
    public void draw(Canvas canvas, CharSequence text, int start, int end, float x, int top, int y, int bottom,
                     Paint paint) {
        Drawable d = getDrawable();
        Paint.FontMetricsInt pfm = paint.getFontMetricsInt();
        int fontCentreY = y + (pfm.ascent + pfm.descent) / 2; // y is the text baseline
        int transY = fontCentreY - d.getBounds().height() / 2;
        canvas.save();
        canvas.translate(x, transY);
        d.draw(canvas);
        canvas.restore();
    }
}
