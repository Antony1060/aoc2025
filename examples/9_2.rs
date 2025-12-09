use rayon::iter::ParallelIterator;
use rayon::prelude::IntoParallelIterator;
use std::cmp::{max, min};
use std::collections::HashMap;
use std::ops::RangeInclusive;
use std::sync::RwLock;

const INPUT: &str = include_str!("../input.txt");

#[derive(Clone, Debug, Ord, PartialOrd, Eq, PartialEq, Hash)]
struct Coord {
    x: u64,
    y: u64,
}

type Rng = RangeInclusive<u64>;
type MovingRng = (u64, Rng);

#[derive(Debug)]
struct Ctx {
    grid_x: u64,
    grid_y: u64,
    x_ranges: Vec<MovingRng>,
    y_ranges: Vec<MovingRng>,
}

static POINT_DP: once_cell::sync::Lazy<RwLock<HashMap<Coord, bool>>> =
    once_cell::sync::Lazy::new(|| RwLock::new(HashMap::new()));

fn main() {
    let input = INPUT.trim();

    let coords = input
        .split("\n")
        .map(|it| it.split(",").map(|n| n.parse::<u64>().unwrap()))
        .map(|mut it| {
            let y = it.next().unwrap();
            let x = it.next().unwrap();
            Coord { x, y }
        })
        .collect::<Vec<_>>();

    let (x_ranges, y_ranges) = gen_ranges(&coords);

    let ctx = Ctx {
        grid_x: coords.iter().map(|it| it.x).max().unwrap() + 1,
        grid_y: coords.iter().map(|it| it.y).max().unwrap() + 1,
        x_ranges,
        y_ranges,
    };

    let ub = (coords.len() * (coords.len() - 1)) / 2 / 32;
    let mx = (0..(coords.len() - 1))
        .into_par_iter()
        .map({
            |i| {
                let mut mx: u64 = 0;
                for (cnt, j) in ((i + 1)..coords.len()).enumerate() {
                    println!("{i}={cnt}/{}", ub);

                    let c1 = &coords[i];
                    let c2 = &coords[j];

                    if !is_good_square(&ctx, c1, c2) {
                        continue;
                    }

                    let area = (c1.x.abs_diff(c2.x) + 1) * (c1.y.abs_diff(c2.y) + 1);
                    mx = max(mx, area);
                }

                mx
            }
        })
        .max()
        .unwrap();

    println!("{mx}");
}

fn is_in_range(ranges: &[MovingRng], constant: u64, delta: u64) -> bool {
    ranges
        .iter()
        .any(|(c, rng)| *c == constant && rng.contains(&delta))
}

fn is_good_point(ctx: &Ctx, coord: &Coord) -> bool {
    if let Some(ret) = (*POINT_DP).read().unwrap().get(coord) {
        return *ret;
    }

    let mut h1 = false;
    let mut h2 = false;
    let mut h3 = false;
    let mut h4 = false;

    let mut d = 0;

    loop {
        let (dx, dy) = (coord.x + d, coord.y + d);

        if (!h1 && dx >= ctx.grid_x) || (!h2 && dy >= ctx.grid_y) {
            break;
        }

        if !h1 {
            if is_in_range(&ctx.x_ranges, dx, coord.y) {
                h1 = true;
            }
        }
        if !h2 {
            if is_in_range(&ctx.y_ranges, dy, coord.x) {
                h2 = true;
            }
        }

        let (dx, dy) = ((coord.x as i64) - (d as i64), (coord.y as i64) - (d as i64));

        if (!h3 && dx < 0) || (!h4 && dy < 0) {
            break;
        }

        if !h3 {
            if is_in_range(&ctx.x_ranges, dx as u64, coord.y) {
                h3 = true;
            }
        }
        if !h4 {
            if is_in_range(&ctx.y_ranges, dy as u64, coord.x) {
                h4 = true;
            }
        }

        if h1 && h2 && h3 && h4 {
            (*POINT_DP).write().unwrap().insert(coord.clone(), true);

            return true;
        }

        d += 1
    }

    (*POINT_DP).write().unwrap().insert(coord.clone(), false);
    false
}

fn is_good_square(ctx: &Ctx, coord1: &Coord, coord2: &Coord) -> bool {
    for i in min(coord1.x, coord2.x)..=max(coord1.x, coord2.x) {
        if !is_good_point(ctx, &Coord { x: i, y: coord1.y })
            || !is_good_point(ctx, &Coord { x: i, y: coord2.y })
        {
            return false;
        }
    }

    for i in min(coord1.y, coord2.y)..=max(coord1.y, coord2.y) {
        if !is_good_point(ctx, &Coord { x: coord1.x, y: i })
            || !is_good_point(ctx, &Coord { x: coord2.x, y: i })
        {
            return false;
        }
    }

    true
}

fn join_adjacent(
    coords: &[Coord],
    idx1: usize,
    idx2: usize,
    x_ranges: &mut Vec<MovingRng>,
    y_ranges: &mut Vec<MovingRng>,
) {
    let i = &coords[idx1];
    let j = &coords[idx2];
    if i.x == j.x {
        x_ranges.push((i.x, min(i.y, j.y)..=max(i.y, j.y)))
    } else if i.y == j.y {
        y_ranges.push((i.y, min(i.x, j.x)..=max(i.x, j.x)))
    }
}

fn gen_ranges(coords: &[Coord]) -> (Vec<MovingRng>, Vec<MovingRng>) {
    let mut x_ranges = Vec::new();
    let mut y_ranges = Vec::new();

    for c in 0..(coords.len() - 1) {
        join_adjacent(coords, c, c + 1, &mut x_ranges, &mut y_ranges);
    }
    join_adjacent(coords, coords.len() - 1, 0, &mut x_ranges, &mut y_ranges);

    (x_ranges, y_ranges)
}
