// winid — print the on-screen CGWindowID(s) of a process, one per line (normal windows, layer 0).
//
// Used by the macOS/Catalyst parity capture: `screencapture -R` grabs a SCREEN REGION (whatever is on top
// there), so an obscured app window can't be captured that way. `screencapture -l<windowid>` captures the
// specific WINDOW even when obscured — but needs the CGWindowID, which this tool resolves from the app's pid.
// Listing window METADATA (this tool) needs no Screen Recording permission; capturing PIXELS (screencapture)
// does, and the system `screencapture` already has it.
//
// Build (native arm64 macOS, NOT macabi):
//   clang -framework Foundation -framework CoreGraphics tools/parity/winid.m -o build/winid
// Use:
//   WID=$(build/winid <pid> | head -1); screencapture -l$WID -o out.png
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: winid <pid>\n");
        return 2;
    }
    pid_t pid = (pid_t)atoi(argv[1]);
    CFArrayRef wins = CGWindowListCopyWindowInfo(
        kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    for (NSDictionary* w in (__bridge NSArray*)wins)
    {
        if ([w[(__bridge id)kCGWindowOwnerPID] intValue] != pid) continue;
        if ([w[(__bridge id)kCGWindowLayer] intValue] != 0) continue; // skip menubar/panels
        printf("%d\n", [w[(__bridge id)kCGWindowNumber] intValue]);
    }
    CFRelease(wins);
    return 0;
}
