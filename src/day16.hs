import qualified Data.Sequence as Seq
import qualified Data.Bool as Bool
import qualified Data.Bits as Bits
import qualified Data.List as List

dragonCurve :: Int -> Bool
dragonCurve k =
  odd (k `Bits.shiftR` (Bits.countTrailingZeros k + 1))

dragonModified :: Seq.Seq Bool -> Int -> Bool
dragonModified seed i
  | o == l = dragonCurve (k + 1)
  | even k = Seq.index seed o
  | otherwise = Seq.index mirror o
  where
    l = length seed
    (k, o) = i `divMod` (l + 1)
    mirror = Seq.reverse (fmap not seed)

checksum :: Int -> [Bool] -> [Bool]
checksum level = map snd . List.foldl' (step level) []
  where
    step l [] x = [(l, x)]
    step 0 xs x = (0, x) : xs
    step l ((l', a):xs) b
      | l == l' = step (l - 1) xs (a == b)
      | otherwise = (l, b) : (l', a) : xs


asBools :: String -> [Bool]
asBools = map (== '1')

asString :: [Bool] -> String
asString = map (Bool.bool '0' '1')

solve :: String -> Int -> String
solve seed disk =
    asString
  . checksum (Bits.countTrailingZeros disk)
  $ map (dragonModified (Seq.fromList $ asBools seed)) [0..disk-1]

main :: IO ()
main = do
  putStrLn $ solve "110010110100" 12
  putStrLn $ solve "10000" 20

  putStrLn $ solve "11100010111110100" 272
  putStrLn $ solve "11100010111110100" 35651584
