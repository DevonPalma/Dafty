#pragma once
// Dispatch once per EFFECT, not per PIXEL. The compiler specializes
// renderEffect<Layout>() and removes irrelevant layout mapping branches.
#include "Animations.h"
inline void renderFlashEffect(const FlashEffect &e, float progress)
{
  switch (e.target)
  {
  case Target::All:
    renderEffect<All>(e, progress);
    break;
  case Target::LeftRing:
    renderEffect<LeftRing>(e, progress);
    break;
  case Target::FacePanel:
    renderEffect<FacePanel>(e, progress);
    break;
  case Target::RightRing:
    renderEffect<RightRing>(e, progress);
    break;
  case Target::BothRings:
    renderEffect<BothRings>(e, progress);
    break;
  case Target::LeftRingShape:
    renderEffect<LeftRing>(e, progress);
    break;
  case Target::FaceMatrix:
    renderEffect<FaceMatrix>(e, progress);
    break;
  case Target::RightRingShape:
    renderEffect<RightRing>(e, progress);
    break;
  }
}
