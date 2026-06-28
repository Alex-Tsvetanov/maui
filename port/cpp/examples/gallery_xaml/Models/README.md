# Models/

Kept to mirror the MAUI project layout (`Views/` + `ViewModels/` + `Models/`), but intentionally empty: the
gallery twins carry no domain model types — their demo data is inline in the markup (`x:Array`). A twin that
introduced a real entity (the item type behind a `CollectionView`, say) would define it here and reference it
from its `ViewModels/` type.
